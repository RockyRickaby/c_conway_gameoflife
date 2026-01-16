#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "gameoflife.h"
#include "arena.h"
#include "set.h"
#include "sizes.h"
// #include "gol.h"

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 800;

int main(int argc, char **argv) {
	// initial random state
	GameOfLife game = {0};
	gameoflife_init(&game);
	gameoflife_cell_birth(&game, 0, 0);
	gameoflife_cell_birth(&game, 1, -1);
	gameoflife_cell_birth(&game, 1, -2);
	gameoflife_cell_birth(&game, 0, -2);
	gameoflife_cell_birth(&game, -1, -2);

	size_t out = 0;
	Point2Di32 *points = NULL;
	Camera2D cam = {0};
	cam.zoom = 10;
	// cam.offset = (Vector2) { .x = -width / 2.0f, .y = -height / 2.0f };
	// cam.target = (Vector2) { .x = -width / 2.0f, .y = -height / 2.0f };

	const float sim_speed = 0.125f;
	float acc = 0;
	int run = 0;
	// TODO - let user control simulation speed
	// TODO - let user pause/unpause simulation
	// TODO - let usar draw on the screen
	// TODO - add dark mode
	// TODO - figure out how to better draw pixels on a screen
	size_t prev_val = 0;
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conway's Game of Life");
	SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN | FLAG_VSYNC_HINT);
	int width = GetMonitorWidth(GetCurrentMonitor());
	int height = GetMonitorHeight(GetCurrentMonitor());

	RenderTexture2D grid = LoadRenderTexture(width, height);
	BeginTextureMode(grid);  // Enable drawing to texture
	    ClearBackground(BLANK);  // Clear texture background
	    DrawCircle(500, 500, 5, RED);
		rlPushMatrix();
			rlTranslatef(500, 500, 0);
			rlRotatef(90, 1, 0, 0);
			DrawGrid(100, 1);
		rlPopMatrix();
	EndTextureMode();
	int update = 1;
	while (!WindowShouldClose()) {
		acc += GetFrameTime();
		if (run && acc > sim_speed) {
			update = gameoflife_step(&game);
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
			Vector2 pos = { 
				.x = mouse_world_pos.x > 0 ? (int)mouse_world_pos.x : floor(mouse_world_pos.x),
				.y = mouse_world_pos.y > 0 ? (int)mouse_world_pos.y : floor(mouse_world_pos.y)
			};
			gameoflife_cell_birth(&game, (int)pos.x, (int)-pos.y);
		}
		if (IsKeyPressed(KEY_SPACE)) {
			run = !run;
		}
	    float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            const float scale = 0.2f * wheel;
			const Vector2 mouse_pos = GetMousePosition();
            const Vector2 mouse_world_pos = GetScreenToWorld2D(mouse_pos, cam);
            cam.offset = mouse_pos;
            cam.target = mouse_world_pos;
            cam.zoom = Clamp(expf(logf(cam.zoom) + scale), 5.0f, 64.0f);
        }
		BeginDrawing();
			ClearBackground(WHITE);
			BeginMode2D(cam);
				// rlPushMatrix();
				if (update) {
					size_t cells = gameoflife_get_cells(&game, &points);
					// TODO - do better
					for (int i = 0; i < cells; i++) {
						DrawPixel(points[i].x, -points[i].y, BLACK);
					}
				}
				// DrawText("Hello, world!!", 100, 100, 32, BLACK);
				// rlPopMatrix();
			EndMode2D();
			// DrawTexture(grid.texture, 0, 0, BLANK);
		EndDrawing();
	}
	UnloadRenderTexture(grid);
	CloseWindow();

	gameoflife_uninit(&game);
}
