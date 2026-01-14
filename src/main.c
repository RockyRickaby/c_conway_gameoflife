#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "arena.h"
#include "set.h"
#include "sizes.h"
#include "gol.h"

int main(int argc, char **argv) {
	int width = 800;
	int height = 600;
	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}
	// initial random state
	GameOfLife game = {0};
	gol_init(&game);
	gol_birth_cell(&game, 0, 0);
	gol_birth_cell(&game, 1, -1);
	gol_birth_cell(&game, 1, -2);
	gol_birth_cell(&game, 0, -2);
	gol_birth_cell(&game, -1, -2);
	// gol_step(&game);
	// Arena *arena = arena_create(MiB(4));
	size_t out = 0;
	// Point2DInt *points = ARENA_PUSH_ARRAY_ZERO();
	Point2DInt *points = NULL;
	Camera2D cam = {0};
	cam.zoom = 1;
	// cam.offset = (Vector2) { .x = -width / 2.0f, .y = -height / 2.0f };
	// cam.target = (Vector2) { .x = -width / 2.0f, .y = -height / 2.0f };

	const float sim_speed = 1.0f;
	float acc = 0;
	// TODO - let user control simulation speed
	// TODO - let user pause/unpause simulation
	// TODO - let usar draw on the screen
	// TODO - add dark mode
	// TODO - figure out how to better draw pixels on a screen
	InitWindow(width, height, "Conway's Game of Life");
	SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN | FLAG_VSYNC_HINT);
	while (!WindowShouldClose()) {
		acc += GetFrameTime();
		if (acc > sim_speed) {
			// gol_step(&game);
			acc = 0;
		}
		// https://www.raylib.com/examples/core/loader.html?name=core_2d_camera_mouse_zoom
       	if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f/cam.zoom);
            cam.target = Vector2Add(cam.target, delta);
        }
		if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
			Vector2 mouse_pos = GetMousePosition();
			Vector2 mouse_world_pos = GetScreenToWorld2D(mouse_pos, cam); // THIS ONE!!
			// Vector2 rounded = {
			// 	.x = (int)round(mouse_world_pos.x / cam.zoom),
			// 	.y = (int)round(mouse_world_pos.y / cam.zoom)
			// };
			printf("(%f,%f)\n", mouse_pos.x, mouse_pos.y);
			printf("(%f,%f)\n", mouse_world_pos.x, mouse_world_pos.y);
			gol_birth_cell(&game, (int)round(mouse_world_pos.x), (int)round(-mouse_world_pos.y));
		}
	    float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            const float scale = 0.2f * wheel;
			const Vector2 mouse_pos = GetMousePosition();
            const Vector2 mouse_world_pos = GetScreenToWorld2D(mouse_pos, cam);
            cam.offset = mouse_pos;
            cam.target = mouse_world_pos;
            cam.zoom = Clamp(expf(logf(cam.zoom) + scale), 1.0f, 64.0f);
        }
		BeginDrawing();
			ClearBackground(WHITE);
			BeginMode2D(cam);
				rlPushMatrix();
					// rlTranslatef(width / 2.0f, height / 2.0f, 0);
					// memset(points, 0, sizeof points);
					size_t cells = gol_num_cells(&game);
					gol_get_cells(&game, points, sizeof points / sizeof *points, &points);
					// TODO - do better
					for (int i = 0; i < cells; i++) {
						DrawPixel(points[i].x, -points[i].y, BLACK);
					}
					// DrawText("Hello, world!!", 100, 100, 32, BLACK);
				rlPopMatrix();
			EndMode2D();
		EndDrawing();
	}
	CloseWindow();
	gol_uninit(&game);
}
