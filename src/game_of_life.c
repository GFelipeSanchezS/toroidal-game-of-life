#include "game_of_life.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int wrap(int v, int size) {
    return ((v % size) + size) % size;
}

GameOfLife *gol_create(int width, int height) {
    if (width <= 0 || height <= 0)
        return NULL;

    GameOfLife *gol = malloc(sizeof(GameOfLife));
    if (!gol)
        return NULL;

    gol->width = width;
    gol->height = height;
    gol->cells = calloc((size_t)width * height, sizeof(bool));
    gol->next = calloc((size_t)width * height, sizeof(bool));

    if (!gol->cells || !gol->next) {
        free(gol->cells);
        free(gol->next);
        free(gol);
        return NULL;
    }

    return gol;
}

void gol_destroy(GameOfLife *gol) {
    if (gol) {
        free(gol->cells);
        free(gol->next);
        free(gol);
    }
}

bool gol_get(const GameOfLife *gol, int x, int y) {
    int wx = wrap(x, gol->width);
    int wy = wrap(y, gol->height);
    return gol->cells[wy * gol->width + wx];
}

static bool get_direct(const GameOfLife *gol, int x, int y) {
    return gol->cells[y * gol->width + x];
}

void gol_set(GameOfLife *gol, int x, int y, bool alive) {
    int wx = wrap(x, gol->width);
    int wy = wrap(y, gol->height);
    gol->cells[wy * gol->width + wx] = alive;
}

int gol_count_neighbors(const GameOfLife *gol, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue;
            int nx = wrap(x + dx, gol->width);
            int ny = wrap(y + dy, gol->height);
            if (get_direct(gol, nx, ny))
                count++;
        }
    }
    return count;
}

void gol_step(GameOfLife *gol) {
    for (int y = 0; y < gol->height; y++) {
        for (int x = 0; x < gol->width; x++) {
            int n = gol_count_neighbors(gol, x, y);
            bool alive = get_direct(gol, x, y);
            bool next_alive;
            if (alive) {
                next_alive = (n == 2 || n == 3);
            } else {
                next_alive = (n == 3);
            }
            gol->next[y * gol->width + x] = next_alive;
        }
    }
    bool *tmp = gol->cells;
    gol->cells = gol->next;
    gol->next = tmp;
}

void gol_clear(GameOfLife *gol) {
    memset(gol->cells, 0, (size_t)gol->width * gol->height * sizeof(bool));
    memset(gol->next, 0, (size_t)gol->width * gol->height * sizeof(bool));
}

void gol_randomize(GameOfLife *gol) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = 1;
    }
    for (int i = 0; i < gol->width * gol->height; i++) {
        gol->cells[i] = (rand() % 10) < 3;
    }
}

void gol_toggle(GameOfLife *gol, int x, int y) {
    int wx = wrap(x, gol->width);
    int wy = wrap(y, gol->height);
    int idx = wy * gol->width + wx;
    gol->cells[idx] = !gol->cells[idx];
}

bool gol_is_empty(const GameOfLife *gol) {
    for (int i = 0; i < gol->width * gol->height; i++) {
        if (gol->cells[i])
            return false;
    }
    return true;
}

unsigned int gol_hash(const GameOfLife *gol) {
    unsigned int h = 2166136261u;
    for (int i = 0; i < gol->width * gol->height; i++) {
        h ^= (unsigned int)gol->cells[i];
        h *= 16777619u;
    }
    return h;
}
