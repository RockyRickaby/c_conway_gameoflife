#pragma once

#include <stdint.h>
#include <stddef.h>

#include "set.h"
#include "arena.h"

#define POINT_TO_U64(x,y) (((uint64_t)(x) << (uint32_t)32) | (uint32_t)(y))
#define U64_TO_POINT(__u64, x, y)\
    do {                                                                                \
        (x) = (int32_t)((uint64_t)(__u64) >> (uint32_t)32);                            \
        (y) = (int32_t)((uint64_t)(__u64) & (uint32_t)0xFFFFFFFF);                     \
    } while (0)

typedef struct _point_2d_i32 {
    int32_t x;
    int32_t y;
} Point2Di32;

typedef struct _game_of_life_s {
    Set cells;
    Arena *arena;
    int generation;
    struct {
        Point2Di32 *list;
        size_t length;
        size_t arena_size_b;
        int requires_update;
    } cells_list;
} GameOfLife;

void gameoflife_init(GameOfLife *game);
void gameoflife_uninit(GameOfLife *game);
size_t gameoflife_living_cells(GameOfLife *game);
// DO NOT FREE THE BUFFER
size_t gameoflife_get_cells(GameOfLife *game, Point2Di32 **out_buf);
void gameoflife_cell_toggle(GameOfLife *game, int x, int y);
int gameoflife_cell_birth(GameOfLife *game, int x, int y);
int gameoflife_cell_kill(GameOfLife *game, int x, int y);
int gameoflife_step(GameOfLife *game);
int gameoflife_steps(GameOfLife *game, int steps);