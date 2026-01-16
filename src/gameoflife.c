#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "gameoflife.h"
#include "arena.h"
#include "sizes.h"
#include "set.h"

static const size_t ARENA_CAP_DEFAULT = MiB(4);
static const size_t SET_CAP_DEFAULT = KiB(4);

struct _cell_s {
    int32_t x;
    int32_t y;
    int alive;
};

void gameoflife_init(GameOfLife *game) {
    memset(game, 0, sizeof(*game));
    set_init(&game->cells, SET_CAP_DEFAULT, NULL);
    game->arena = arena_create(ARENA_CAP_DEFAULT);
    game->generation = 0;
    game->cells_list.list = NULL;
    game->cells_list.length = 0;
    game->cells_list.arena_size_b = 0;
    game->cells_list.requires_update = 0;
    return;
}

void gameoflife_uninit(GameOfLife *game) {
    arena_destroy(game->arena);
    set_uninit(&game->cells);
    memset(game, 0, sizeof(*game));
    return;
}

size_t gameoflife_living_cells(GameOfLife *game) {
    return set_size(&game->cells);
}

size_t gameoflife_get_cells(GameOfLife *game, Point2Di32 **out_buf) {
    if (!game->cells_list.requires_update) {
        *out_buf = game->cells_list.list;
        return game->cells_list.length;
    }
    
    game->cells_list.requires_update = 0;
    size_t cells_cap = set_capacity(&game->cells);
    size_t cells_size = set_size(&game->cells);
    if (game->cells_list.list != NULL) {
        arena_pop(game->arena, game->cells_list.arena_size_b);
        game->cells_list.list = NULL;
        game->cells_list.length = 0;
        game->cells_list.arena_size_b = 0;
    }
    game->cells_list.list = ARENA_PUSH_ARRAY_ZERO(game->arena, Point2Di32, cells_size, &game->cells_list.arena_size_b);
    size_t count = 0;
    Entry *entries = set_entries(&game->cells);
    for (size_t i = 0; i < cells_cap; i++) {
        if (entries[i].free == S_OCCUPIED) {
            U64_TO_POINT(entries[i].keyval, game->cells_list.list[count].x, game->cells_list.list[count].y);
            count++;
            if (count == cells_size) {
                break;
            }
        }
    }
    // == cells_size
    game->cells_list.length = count;
    *out_buf = game->cells_list.list;
    return count;
}

void gameoflife_cell_toggle(GameOfLife *game, int x, int y) {
    if (!set_put(&game->cells, POINT_TO_U64(x,y))) {
        set_remove(&game->cells, POINT_TO_U64(x,y));
    }
    game->cells_list.requires_update = 1;
}

int gameoflife_cell_birth(GameOfLife *game, int x, int y) {
    game->cells_list.requires_update = 1;
    return set_put(&game->cells, POINT_TO_U64(x,y));
}

int gameoflife_cell_kill(GameOfLife *game, int x, int y) {
    game->cells_list.requires_update = 1;
    return set_remove(&game->cells, POINT_TO_U64(x,y));
}

static inline int count_neighbors(GameOfLife *game, int32_t x, int32_t y) {
    int min = -1;
    int max = 1;
    int count = 0;
    for (int dy = min; dy <= max; dy++) {
        for (int dx = min; dx <= max; dx++) {
            if (dx == 0 && dy == 0) continue; 
            if (set_contains(&game->cells, POINT_TO_U64(x + dx, y + dy))) {
                count++;
            }
        }
    }
    return count;
}

int gameoflife_step(GameOfLife *game) {
    int has_next = 0;
    size_t cells_cap = set_capacity(&game->cells);
    // size_t cells_size = set_size(&game->cells);
    size_t count = 0;
    size_t size_out = 0;
    Entry *entries = set_entries(&game->cells);
    struct _cell_s *new_cells = ARENA_PUSH_ARRAY_ZERO(game->arena, struct _cell_s, cells_cap * 8, &size_out);
    
    for (size_t i = 0; i < cells_cap; i++) {
        if (entries[i].free == S_OCCUPIED) {
            int32_t x;
            int32_t y;
            U64_TO_POINT(entries[i].keyval, x, y);
            int min = -1;
            int max = 1;
            int neighbors = count_neighbors(game, x, y);
            // 
            if (neighbors == 2 || neighbors == 3) {
                new_cells[count] = (struct _cell_s) {
                    .x = x,
                    .y = y,
                    .alive = 1
                };
                count++;
            }
            for (int dy = min; dy <= max; dy++) {
                for (int dx = min; dx <= max; dx++) {
                    if ((dy == 0 && dx == 0) || set_contains(&game->cells, POINT_TO_U64(x + dx, y + dy))) {
                        continue;
                    }
                    neighbors = count_neighbors(game, x + dx, y + dy);
                    if (neighbors == 3) {
                        new_cells[count] = (struct _cell_s) {
                            .x = x + dx,
                            .y = y + dy,
                            .alive = 1
                        };
                        count++;
                    }
                }
            }
        }
    }

    set_clear(&game->cells);
    has_next = count > 0;
    for (size_t i = 0; i < count; i++) {
        set_put(&game->cells, POINT_TO_U64(new_cells[i].x,new_cells[i].y));
    }

    game->cells_list.requires_update = game->cells_list.requires_update ? 1 : has_next;
    arena_pop(game->arena, size_out);
    return has_next;
}

int gameoflife_steps(GameOfLife *game, int steps) {
    int ret = 0;
    while (steps--) ret = gameoflife_step(game);
    return ret;
}
