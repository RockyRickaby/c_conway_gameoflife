#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <align.h>
#include <stdint.h>
#include <stddef.h>

#include "gol.h"
#include "set.h"
#include "arena.h"
#include "align.h"
#include "sizes.h"

#define POINT_TO_U64(x,y) (((uint64_t)(x) << (uint32_t)32) | (uint32_t)(y))
#define U64_TO_POINT(__u64, x, y)\
    do {                                                                                \
        (x) = (int32_t)((uint64_t)(__u64) >> (uint32_t)32);                            \
        (y) = (int32_t)((uint64_t)(__u64) & (uint32_t)0xFFFFFFFF);                     \
    } while (0)

// static struct {
//     Point2DInt *ls;
//     size_t ls_cap;
//     size_t ls_size;
//     size_t size_in_arena;
// } Cells = {
//     .ls = NULL,
//     .ls_size = 0,
//     .ls_cap = 0,
//     .size_in_arena = 0,
// };

struct _cell_s {
    Point2DInt point;
    int alive;
};

static const size_t ARENA_CAP_DEFAULT = MiB(4);
static const size_t SET_CAP_DEFAULT = KiB(4);

// static Set __private_set = {0};
// static Set *cells_set = &__private_set;
// static Arena *arena = NULL;
// static size_t arena_cap = ARENA_CAP_DEFAULT;
// static size_t set_cap = SET_CAP_DEFAULT;

static inline void resize_cells_ls(GameOfLife *gol) {
    arena_pop(gol->arena, gol->cells.size_in_arena);
    gol->cells.ls_cap = set_capacity(&gol->cells_set);
    gol->cells.ls = ARENA_PUSH_ARRAY_ZERO(gol->arena, Point2DInt, gol->cells.ls_cap, &gol->cells.size_in_arena);
    gol->cells.ls_size = 0;
}

// static int initialized = 0;
int gol_init(GameOfLife *gol) {
    if (gol->initialized) {
        return 1;
    }

    gol->cells.requires_update = 1;
    gol->arena_cap = ARENA_CAP_DEFAULT;
    gol->cells_cap = SET_CAP_DEFAULT;
    set_init(&gol->cells_set, gol->cells_cap, NULL);
    gol->arena = arena_create(gol->arena_cap);
    gol->initialized = 1;
    
    // no need to pop when the game is uninitialized. the arena will just free it
    // resize whenever the set resizes
    gol->cells.ls_cap = set_capacity(&gol->cells_set);
    gol->cells.ls = ARENA_PUSH_ARRAY_ZERO(gol->arena, Point2DInt, gol->cells.ls_cap, &gol->cells.size_in_arena);
    gol->cells.ls_size = 0;
    return 0;
}

void gol_uninit(GameOfLife *gol) {
    if (gol->initialized) {
        set_uninit(&gol->cells_set);
        arena_destroy(gol->arena);
        gol->initialized = 0;

        gol->cells_set = (Set){0};
        gol->arena = NULL;
        gol->cells.ls = NULL;
        gol->cells.ls_size = 0;
        gol->cells.ls_cap = 0;
    }
}

size_t gol_num_cells(const GameOfLife *gol) {
    return set_size(&gol->cells_set);
}

int gol_get_cells(GameOfLife *gol, Point2DInt *out_buf, size_t buf_size, Point2DInt **copy) {
    // TODO - fix. this doesn't work for some reason
    if (!gol->cells.requires_update) {
        if (copy) {
            *copy = gol->cells.ls;
        } else {
            memcpy(out_buf, gol->cells.ls, (gol->cells.ls_size > buf_size ? buf_size : gol->cells.ls_size) * sizeof(Point2DInt));
        }
        return 0;
    };
    gol->cells.requires_update = 0;
    if (set_capacity(&gol->cells_set) > gol->cells.ls_cap) resize_cells_ls(gol);
    Entry *ls = set_entries(&gol->cells_set);
    size_t count = 0;
    size_t cap = set_capacity(&gol->cells_set);
    size_t size = set_size(&gol->cells_set);
    for (size_t i = 0; i < cap && i < gol->cells.ls_cap; i++) {
        if (ls[i].free == S_OCCUPIED) {
            // U64_TO_POINT(ls[i].keyval, out_buf[count].x, out_buf[count].y);
            U64_TO_POINT(ls[i].keyval, gol->cells.ls[count].x, gol->cells.ls[count].y);
            // printf("%d, %d\n", gol->cells.ls[count].x, gol->cells.ls[count].y);
            count++;
            if (count == size) {
                break;
            }
        }
    }
    gol->cells.ls_size = count;
    if (copy) {
        *copy = gol->cells.ls;
    } else {
        memcpy(out_buf, gol->cells.ls, count * sizeof(Point2DInt));
    }
    return 0;
}

Set *gol_get_set(GameOfLife *gol) {
    return &gol->cells_set;
}

void gol_birth_cell(GameOfLife *gol, int32_t x, int32_t y) {
    // printf("%lu\n", POINT_TO_U64(x,y));
    gol->cells.requires_update = set_put(&gol->cells_set, POINT_TO_U64(x,y)) ? 1 : gol->cells.requires_update;
}

void gol_kill_cell(GameOfLife *gol, int32_t x, int32_t y) {
    gol->cells.requires_update = set_remove(&gol->cells_set, POINT_TO_U64(x, y)) ? 1 : gol->cells.requires_update;
}

void gol_add_cells(GameOfLife *gol, const Point2DInt *ls, size_t len) {
    for (size_t i = 0; i < len; i++) {
        // gol_birth_cell(ls[i].x, ls[i].y);
        set_put(&gol->cells_set, POINT_TO_U64(ls[i].x, ls[i].y));
    }
}

static struct _cell_s *ls = NULL;
static size_t ls_idx = 0;
static size_t ls_cap = 0;
// base case -> current cell is dead
// recursive case -> current cell is alive and at least one neighbor is dead
static void update_cell(GameOfLife *gol, int32_t x, int32_t y);

int gol_step(GameOfLife *gol) {
    if (set_size(&gol->cells_set) == 0) {
        return 0; // stoppped
    }
    size_t bytes = 0;
    ls_cap = (size_t)(gol->cells_set.capacity * 1.5);
    ls_idx = 0;
    ls = ARENA_PUSH_ARRAY_ZERO(gol->arena, struct _cell_s, ls_cap, &bytes);
    for (size_t i = 0; i < gol->cells_set.capacity; i++) {
        if (gol->cells_set.entries[i].free == S_OCCUPIED) {
            int32_t x, y;
            U64_TO_POINT(gol->cells_set.entries[i].keyval, x, y);
            update_cell(gol, x, y);
        }
    }
    int has_next_state = 0;
    for (size_t i = 0; i < ls_idx; i++) {
        if (ls[i].alive) {
            set_put(&gol->cells_set, POINT_TO_U64(ls[i].point.x, ls[i].point.y));
            has_next_state = 1;
        } else {
            set_remove(&gol->cells_set, POINT_TO_U64(ls[i].point.x, ls[i].point.y));
            has_next_state = 1;
        }
    }
    gol->cells.requires_update = gol->cells.requires_update ? 1 : has_next_state;
    arena_pop(gol->arena, bytes); // discard array of cells
    ls = NULL;
    return has_next_state;
}

int gol_steps(GameOfLife *gol, int steps) {
    while (steps--) gol_step(gol);
    return 0;
}

static void update_cell(GameOfLife *gol, int32_t x, int32_t y) {
    if (ls_idx >= ls_cap) { // not the base case. just prevents out of bounds access
        return;
    }
    const int min = -1;
    const int max = 1;

    int neighbors = 0;
    int alive = set_contains(&gol->cells_set, POINT_TO_U64(x, y));
    for (int dy = min; dy <= max; dy++) {
        for (int dx = min; dx <= max; dx++) {
            if (dy == 0 && dx == 0) {
                continue;
            }
            // neighbor is alive
            int neighbor_is_alive = set_contains(&gol->cells_set, POINT_TO_U64(x + dx, y + dy));
            if (neighbor_is_alive) {
                // printf("(%d,%d) -> has neighbor\n", x, y);
                neighbors++;
            }
            // neighbor is dead, but current cell is alive.
            // check the dead cell and its neighbors
            if (!neighbor_is_alive && alive) {
                update_cell(gol, x + dx, y + dy);
            }
        }
    }

    // we're alive. check if we should die
    if (alive) {
        if (neighbors < 2 || neighbors > 3) {
            ls[ls_idx].alive = 0;
            ls[ls_idx].point.x = x;
            ls[ls_idx].point.y = y;
            ls_idx++;
        } else {
            ls[ls_idx].alive = 1;
            ls[ls_idx].point.x = x;
            ls[ls_idx].point.y = y;
            ls_idx++;
        }
    } else { // we are dead. check if we should live
        if (neighbors == 3) {
            ls[ls_idx].alive = 1;
            ls[ls_idx].point.x = x;
            ls[ls_idx].point.y = y;
            ls_idx++;
        } else {
            ls[ls_idx].alive = 0;
            ls[ls_idx].point.x = x;
            ls[ls_idx].point.y = y;
            ls_idx++;
        }
    }
}