#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    bool *cells;
    bool *next;
    int width;
    int height;
} GameOfLife;

/* Initialize grid with given dimensions. Uses toroidal wrapping. */
GameOfLife *gol_create(int width, int height);

void gol_destroy(GameOfLife *gol);

/* Get cell state with toroidal wrapping (out-of-bounds wrap around) */
bool gol_get(const GameOfLife *gol, int x, int y);

/* Set cell state (wraps indices) */
void gol_set(GameOfLife *gol, int x, int y, bool alive);

/* Count living neighbors with toroidal wrapping */
int gol_count_neighbors(const GameOfLife *gol, int x, int y);

/* Advance simulation by one generation */
void gol_step(GameOfLife *gol);

/* Clear all cells */
void gol_clear(GameOfLife *gol);

/* Fill with random pattern (approx. density 0.3) */
void gol_randomize(GameOfLife *gol);

/* Toggle cell at wrapped coordinates */
void gol_toggle(GameOfLife *gol, int x, int y);

/* Check if no cells are alive */
bool gol_is_empty(const GameOfLife *gol);

/* Hash of current state for loop detection (FNV-1a) */
unsigned int gol_hash(const GameOfLife *gol);

#endif /* GAME_OF_LIFE_H */
