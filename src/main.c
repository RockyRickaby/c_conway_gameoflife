#include <stdio.h>
#include <stdlib.h>
#include <stdalign.h>
#include <inttypes.h> // PRIu64
#include <string.h>
#include <math.h>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "gameoflife.h"
#include "arena.h"
#include "set.h"
#include "sizes.h"

static const int WINDOW_WIDTH = 800;
static const int WINDOW_HEIGHT = 800;

int main(int argc, char **argv) {
	GameOfLife game = {0};
	gameoflife_init(&game);

	Point2Di32 *points = NULL;
	Camera2D cam = {0};
	cam.zoom = 10;
	cam.offset = (Vector2) { .x = 0, .y = 0 };
	cam.target = (Vector2) { .x = 0, .y = 0 };

	float sim_speed = 0.1f;
	float acc = 0;
	int run = 0;
	int instructions = 1;
	// TODO - let user control simulation speed
	// TODO - add dark mode
	// TODO - figure out how to better draw pixels on a screen
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Conway's Game of Life");
	SetWindowState(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_ALWAYS_RUN | FLAG_VSYNC_HINT);
	int width = GetMonitorWidth(GetCurrentMonitor());
	int height = GetMonitorHeight(GetCurrentMonitor());

	// RenderTexture2D grid = LoadRenderTexture(width, height);
	// BeginTextureMode(grid);  // Enable drawing to texture
	//     ClearBackground(BLANK);  // Clear texture background
	//     DrawCircle(500, 500, 5, RED);
	// 	rlPushMatrix();
	// 		rlTranslatef(500, 500, 0);
	// 		rlRotatef(90, 1, 0, 0);
	// 		DrawGrid(100, 1);
	// 	rlPopMatrix();
	// EndTextureMode();
	// int update = 1;

	width = WINDOW_WIDTH;
	height = WINDOW_HEIGHT;
  
	Vector2 top_left_world;
	Vector2 bottom_right_world;

	Rectangle cam_bounds;

	int dark_mode = 0;
	Color background = WHITE;
	Color foreground = BLACK;

	while (!WindowShouldClose()) {
		acc += GetFrameTime();
		if (acc > sim_speed) {
			if (run) {
				gameoflife_step(&game);
			}
			acc = 0;
		}

		width = GetScreenWidth();
		height = GetScreenHeight();

		top_left_world = GetScreenToWorld2D((Vector2) { 0, 0 }, cam);
		bottom_right_world = GetScreenToWorld2D((Vector2) { width, height }, cam);

		cam_bounds = (Rectangle) {
			top_left_world.x,
			top_left_world.y,
			bottom_right_world.x - top_left_world.x,
			bottom_right_world.y - top_left_world.y
		};

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
			// the screen grows downwards, but the game grows upwards
			// flip the sign of Y and everything is good again
			gameoflife_cell_birth(&game, (int)pos.x, (int)-pos.y);
		}

		switch (GetKeyPressed()) {
			case KEY_SPACE: { run = !run; break; }
			case KEY_B: {
				dark_mode = !dark_mode;
				if (dark_mode) {
					foreground = RAYWHITE;
					background = BLACK;
				} else {
					background = RAYWHITE;
					foreground = BLACK;
				}
				break;
			}
			case KEY_C: { gameoflife_clear(&game); break; }
			case KEY_H: { instructions = !instructions; break; }
			default: break;
		}

		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			if (IsKeyPressed(KEY_EQUAL)) {
				sim_speed += 0.05f;
			} else if (IsKeyPressed(KEY_MINUS)) {
				sim_speed -= 0.05f;
			}
			sim_speed = Clamp(sim_speed, 0.05f, 1.0f);
			printf("%.5f\n", sim_speed);
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
		size_t cells = gameoflife_get_cells(&game, &points);
		BeginDrawing();
			// DrawText("sEX", 0, 0, 26, RED);
			ClearBackground(background);
			BeginMode2D(cam);
				// rlPushMatrix();
				// TODO - do better
				for (size_t i = 0; i < cells; i++) {
					Vector2 point = { .x = points[i].x, .y = -points[i].y };
					if (CheckCollisionPointRec(point, cam_bounds)) {
						DrawPixel(point.x, point.y, foreground);
					}
				}
				// DrawText("Hello, world!!", 100, 100, 32, BLACK);
				// rlPopMatrix();
			EndMode2D();
			if (instructions) {
				DrawText("Left Mouse: Add cell\nRight Mouse: Move the camera\n\nSPACE: Runs the simulation\nB: Switches to dark/light mode\nC: Removes all the cells\nH: Show/hide this text", 10, 10, 26, foreground);
			}
			// DrawTexture(grid.texture, 0, 0, BLANK);
		EndDrawing();
		// DrawFPS(0,0);
	}
	// UnloadRenderTexture(grid);
	CloseWindow();

	gameoflife_uninit(&game);
}
