#include <SDL2/SDL.h>
#include <stdio.h>
#include "../include/game_of_life.h"

#define WINDOW_TITLE "Toroidal Game of Life"
#define CELL_WIDTH   8
#define CELL_HEIGHT  8
#define GRID_W       100
#define GRID_H       100
#define BG_R         24
#define BG_G         24
#define BG_B         32
#define CELL_R       64
#define CELL_G       200
#define CELL_B       128

static const Uint32 MIN_FRAME_MS = 33;
#define DEFAULT_STEP_INTERVAL 100
#define HASH_HISTORY_SIZE     64
#define UNIV_W               (GRID_W * CELL_WIDTH)
#define UNIV_H               (GRID_H * CELL_HEIGHT)
#define PAN_SPEED             0.02f
#define PAN_UPDATE_MS         16

static int wrap_int(int v, int size) {
    return ((v % size) + size) % size;
}

static void screen_to_cell(int sx, int sy, float pan_x, float pan_y, int *out_gx, int *out_gy) {
    float wx = (float)sx + pan_x;
    float wy = (float)sy + pan_y;
    *out_gx = wrap_int((int)(wx / CELL_WIDTH), GRID_W);
    *out_gy = wrap_int((int)(wy / CELL_HEIGHT), GRID_H);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    int win_w = UNIV_W;
    int win_h = UNIV_H;

    SDL_Window *win = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        win_w, win_h,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    GameOfLife *gol = gol_create(GRID_W, GRID_H);
    if (!gol) {
        fprintf(stderr, "Failed to create game state\n");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    int paused = 1;
    Uint32 last_step = SDL_GetTicks();
    Uint32 step_interval = DEFAULT_STEP_INTERVAL;
    int draw_mode = 1; /* 1 = draw, 0 = erase */
    int mouse_down = 0;
    int edit_cooldown = 0; /* skip steps for a few frames after mouse release */
    int pan_stopped = 0;   /* 1 = user stopped auto-pan */

    float pan_x = 0;
    float pan_y = 0;
    int pan_dir_y = 1;     /* 1 = +Y (down), -1 = -Y (up); X always goes right */
    Uint32 last_pan_ms = SDL_GetTicks();

    unsigned int hash_history[HASH_HISTORY_SIZE];
    int hash_count = 0;
    int hash_head = 0;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = 0;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                win_w = e.window.data1;
                win_h = e.window.data2;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        running = 0;
                        break;
                    case SDLK_SPACE:
                        paused = !paused;
                        break;
                    case SDLK_c:
                        gol_clear(gol);
                        hash_count = 0;
                        hash_head = 0;
                        break;
                    case SDLK_r:
                        gol_randomize(gol);
                        hash_count = 0;
                        hash_head = 0;
                        paused = 0;
                        break;
                    case SDLK_d:
                        draw_mode = 1;
                        break;
                    case SDLK_e:
                        draw_mode = 0;
                        break;
                    case SDLK_UP:
                        step_interval = step_interval > 20 ? step_interval - 20 : 20;
                        break;
                    case SDLK_DOWN:
                        step_interval = step_interval < 500 ? step_interval + 20 : 500;
                        break;
                    case SDLK_s:
                        step_interval = DEFAULT_STEP_INTERVAL;
                        break;
                    case SDLK_p:
                        pan_stopped = !pan_stopped;
                        break;
                    default:
                        break;
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                mouse_down = 1;
                int gx, gy;
                screen_to_cell(e.button.x, e.button.y, pan_x, pan_y, &gx, &gy);
                int erase = (e.button.button == SDL_BUTTON_RIGHT) ? 1 : !draw_mode;
                gol_set(gol, gx, gy, !erase);
            }
            if (e.type == SDL_MOUSEBUTTONUP)
                mouse_down = 0;
        }

        if (mouse_down) {
            int mx, my;
            Uint32 buttons = SDL_GetMouseState(&mx, &my);
            int gx, gy;
            screen_to_cell(mx, my, pan_x, pan_y, &gx, &gy);
            int erase = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) ? 1 : !draw_mode;
            gol_set(gol, gx, gy, !erase);
        }

        if (edit_cooldown > 0)
            edit_cooldown--;

        /* Auto-pan: always right (+X), zigzag up and down in Y */
        {
            Uint32 now = SDL_GetTicks();
            if (pan_stopped) {
                last_pan_ms = now; /* keep current so dt is small when we resume */
            } else {
                float dt = (float)(now - last_pan_ms);
                if (dt > 100) dt = 100; /* cap to avoid jump when window was in background */
                if (dt > 0) {
                    float move = PAN_SPEED * dt;
                    pan_x += move;                    /* always pan right */
                    if (pan_x >= UNIV_W) pan_x -= UNIV_W;
                    pan_y += pan_dir_y > 0 ? move : -move;  /* zigzag: down then up */
                    if (pan_y >= UNIV_H) { pan_y = UNIV_H; pan_dir_y = -1; }
                    if (pan_y <= 0)      { pan_y = 0;       pan_dir_y = 1;  }
                    last_pan_ms = now;
                }
            }
        }

        if (!paused && !mouse_down && edit_cooldown == 0) {
            Uint32 now = SDL_GetTicks();
            if (now - last_step >= step_interval) {
                gol_step(gol);
                last_step = now;

                if (gol_is_empty(gol)) {
                    gol_randomize(gol);
                    hash_count = 0;
                    hash_head = 0;
                } else {
                    unsigned int h = gol_hash(gol);
                    int loop_found = 0;
                    for (int i = 0; i < hash_count; i++) {
                        if (hash_history[i] == h) {
                            loop_found = 1;
                            break;
                        }
                    }
                    if (loop_found) {
                        gol_randomize(gol);
                        hash_count = 0;
                        hash_head = 0;
                    } else {
                        if (hash_count < HASH_HISTORY_SIZE) {
                            hash_history[hash_count++] = h;
                        } else {
                            hash_history[hash_head] = h;
                            hash_head = (hash_head + 1) % HASH_HISTORY_SIZE;
                        }
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(ren, BG_R, BG_G, BG_B, 255);
        SDL_RenderClear(ren);

        /* Tiled rendering: draw universe repeated to fill viewport */
        int tx_min = (int)(pan_x / UNIV_W);
        int tx_max = (int)((pan_x + win_w) / UNIV_W);
        int ty_min = (int)(pan_y / UNIV_H);
        int ty_max = (int)((pan_y + win_h) / UNIV_H);
        SDL_SetRenderDrawColor(ren, CELL_R, CELL_G, CELL_B, 255);
        for (int ty = ty_min; ty <= ty_max; ty++) {
            for (int tx = tx_min; tx <= tx_max; tx++) {
                int base_x = tx * UNIV_W - (int)pan_x;
                int base_y = ty * UNIV_H - (int)pan_y;
                for (int y = 0; y < gol->height; y++) {
                    for (int x = 0; x < gol->width; x++) {
                        if (gol_get(gol, x, y)) {
                            SDL_Rect r = {
                                base_x + x * CELL_WIDTH + 1,
                                base_y + y * CELL_HEIGHT + 1,
                                CELL_WIDTH - 1,
                                CELL_HEIGHT - 1
                            };
                            SDL_RenderFillRect(ren, &r);
                        }
                    }
                }
            }
        }

        SDL_RenderPresent(ren);

        Uint32 elapsed = SDL_GetTicks() - last_step;
        if (elapsed < MIN_FRAME_MS)
            SDL_Delay(MIN_FRAME_MS - elapsed);
    }

    gol_destroy(gol);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
