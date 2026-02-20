# Toroidal Game of Life

A real-time simulation of [Conway's Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) with a **toroidal** universe: the field wraps on both axes like a torus, so cells at the edges connect to cells on the opposite side.

Built with C and [SDL2](https://www.libsdl.org/).

![License](https://img.shields.io/badge/license-MIT-blue.svg)

## Features

- **Toroidal wrapping** — Top/bottom and left/right edges connect, creating seamless patterns
- **Interactive drawing** — Click and drag to draw or erase cells
- **Speed control** — Adjust simulation speed with keyboard; reset to default with **S**
- **Random patterns** — Fill the grid with random starting states
- **Auto-renewal** — When all cells die or a repeating loop is detected, the grid is automatically filled with a new random pattern

## Requirements

- C compiler (gcc or clang)
- [SDL2](https://www.libsdl.org/download-2.php) development libraries

### Installing SDL2

**Ubuntu / Debian:**
```bash
sudo apt-get install libsdl2-dev
```

**Fedora:**
```bash
sudo dnf install SDL2-devel
```

**Arch Linux:**
```bash
sudo pacman -S sdl2
```

**macOS (Homebrew):**
```bash
brew install sdl2
```

## Building

```bash
make
```

Optional shortcuts:
```bash
make run          # Build and run
make clean        # Remove build artifacts
make install-deps-ubuntu   # Install SDL2 on Ubuntu
make install-deps-fedora   # Install SDL2 on Fedora
```

## Running

```bash
./gameoflife
```

## Controls

| Key | Action |
|-----|--------|
| **Space** | Pause / Resume simulation |
| **Left click** | Add cells (or draw when in draw mode) |
| **Right click** | Remove cells (or erase when in erase mode) |
| **D** | Draw mode (default) — for single-button input |
| **E** | Erase mode — for single-button input |
| **R** | Randomize grid and start running |
| **C** | Clear all cells |
| **↑** | Increase speed (shorter step interval) |
| **↓** | Decrease speed (longer step interval) |
| **S** | Reset speed to default |
| **P** | Stop / Resume auto-pan |
| **Q** / **Esc** | Quit |

**Left-click and drag** to add cells; **right-click and drag** to remove them. The **D** and **E** keys switch mode for single-button devices (e.g. touchpad).

### Window and view

The window is resizable. The universe and cell size stay fixed; the view **tiles** to fill the space:

- **Larger window** — The universe is repeated to fill the window; you see multiple tiles.
- **Smaller window** — You see one tile or a cropped region.

The view **auto-pans** in a zigzag toward the right: it always moves right (+X) while simultaneously oscillating up and down in Y. The result is a diagonal zigzag path toward the right that covers the full universe over time. Press **P** to stop or resume panning. Panning cannot be manually controlled.

### Auto-renewal

The simulation automatically fills the grid with a new random pattern when:

- **All cells die** — The entire population goes extinct (does not trigger when you press **C** to clear manually).
- **A loop is detected** — The pattern enters a repeating cycle (e.g., still lifes, oscillators).

In both cases, the grid is repopulated in the same way as pressing **R**, and the simulation continues running.

## Terminology

| Term | Definition |
|------|------------|
| **Universe** | The 2D space where the simulation runs — the grid of all cells. |
| **Cells** | The individual units of the universe. Each cell is either alive or dead. |
| **Generation** | A single time step of the simulation. Each advance to the next state counts as one generation. |
| **Pattern** | A specific arrangement of cells (e.g., glider, blinker, block, still life). |
| **Neighborhood** | The eight cells surrounding a given cell (Moore neighborhood). Rules are applied using neighbor counts. |
| **Toroidal** | A topology where the universe wraps around: the top edge connects to the bottom, and the left edge connects to the right, like a torus (doughnut shape). |

## Project Structure

```
toroidal-game-of-life/
├── README.md
├── LICENSE
├── Makefile
├── .gitignore
├── include/
│   └── game_of_life.h      # Game logic API
└── src/
    ├── main.c              # SDL window, rendering, input
    └── game_of_life.c      # Toroidal grid and Conway rules
```

## Toroidal Wrapping

In a standard 2D universe, cells at the edges have fewer neighbors. In this project, the universe is **toroidal**: the top row wraps to the bottom, and the left column wraps to the right.

- Neighbor count uses `(x ± 1) % width` and `(y ± 1) % height`
- Gliders and other patterns can cross the screen and re-appear on the opposite side

## Conway's Rules

- A **living** cell with 2 or 3 living neighbors **survives**
- A **living** cell with fewer than 2 or more than 3 neighbors **dies**
- A **dead** cell with exactly 3 living neighbors **becomes alive**

## License

MIT License — see [LICENSE](LICENSE) for details.
