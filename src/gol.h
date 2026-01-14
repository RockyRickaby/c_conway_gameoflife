#pragma once

#include <stdint.h>
#include <stddef.h>

#include "set.h"
#include "arena.h"

typedef struct _point2d_i {
    int32_t x;
    int32_t y;
} Point2DInt;

typedef struct _gol_s {
    Set cells_set;
    Arena *arena;
    size_t arena_cap;
    size_t cells_cap;
    struct {
        Point2DInt *ls;
        size_t ls_cap;
        size_t ls_size;
        size_t size_in_arena;
        int requires_update;
    } cells;
    int generation;
    int initialized;
} GameOfLife;

int gol_init(GameOfLife *gol);
void gol_uninit(GameOfLife *gol);

// returns the amount of living cells in the game
size_t gol_num_cells(const GameOfLife *gol);
// might be more useful to just grab the internal Set*. Avoids having to allocate a buffer
int gol_get_cells(GameOfLife *gol, Point2DInt *out_buf, size_t buf_size, Point2DInt **copy);
// returns the internal Set* used to store the state of the game
Set *gol_get_set(GameOfLife *gol);

// adds (births) a living cell to the game. The world is effectively infinite, so x and y may be whatever
void gol_birth_cell(GameOfLife *gol, int32_t x, int32_t y);
// removes (kills) a cell from the game. The world is effectively infinite, so x and y may be whatever
void gol_kill_cell(GameOfLife *gol, int32_t x, int32_t y);
// adds (births) a collection of living cells to the game 
void gol_add_cells(GameOfLife *gol, const Point2DInt *ls, size_t len);

// run a single step of the simulation
int gol_step(GameOfLife *gol);
// runs multiple steps of the simulation (up to STEPS amount)
int gol_steps(GameOfLife *gol, int steps);

// returns the minimal and maximal points present in the game.
// that is, the point more to the right and up, and the one more to the left and down
// void gol_get_limits(GameOfLife *gol, Point2DInt *min, Point2DInt *max);