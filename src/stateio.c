#include "stateio.h"
#include "coords.h"
#include "state.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void State_translate(struct State* state, enum Direction direction)
{
    bool tiles[GRID_SIZE][GRID_SIZE];
    uint8_t stacks[GRID_SIZE][GRID_SIZE];
    memcpy(tiles, state->tiles, sizeof(bool) * GRID_SIZE * GRID_SIZE);
    memcpy(stacks, state->stacks, sizeof(uint8_t) * GRID_SIZE * GRID_SIZE);

    struct Coords src;
    struct Coords dest;
    for (src.q = 0; src.q < GRID_SIZE; src.q++) {
        for (src.r = 0; src.r < GRID_SIZE; src.r++) {
            dest = src;
            Coords_move(&dest, direction);

            state->tiles[dest.q][dest.r] = tiles[src.q][src.r];
            state->stacks[dest.q][dest.r] = stacks[src.q][src.r];
        }
    }

    for (int p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->active_stacksc[i]; i++) {
            Coords_move(&state->active_stacks[p][i], direction);
        }
    }
}

void State_normalize(struct State* state)
{
    // A state is normalized if both the q=0 and r=0 axes have a tile on
    // them, and q=GRID_SIZE-1, etc., doesn't (unless it must TODO)
    // TODO or we could check for non-wrapping continuity

    // Check for tiles on q=0
    bool tile_on_axis = false;
    for (int r = 0; r < GRID_SIZE && !tile_on_axis; r++) {
        tile_on_axis = state->tiles[0][r];
    }
    if (!tile_on_axis) {
        State_translate(state, NORTHWEST);
        return State_normalize(state);
    }

    // Check for tiles on r=0
    tile_on_axis = false;
    for (int q = 0; q < GRID_SIZE && !tile_on_axis; q++) {
        tile_on_axis = state->tiles[q][0];
    }
    if (!tile_on_axis) {
        State_translate(state, NORTHEAST);
        return State_normalize(state);
    }

    // Check for a gap between tiles
    bool tile_gap = false;
    for (int q = 0; q < GRID_SIZE; q++) {
        bool empty_column = true;
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->tiles[q][r]) {
                empty_column = false;
                break;
            }
        }
        if (empty_column) {
            tile_gap = true;
            break;
        }
    }

    // If there's a tile gap, the tiles are not stretched across the
    // full grid and thus shouldn't be against the far edges (in other
    // words, check for wrapping)
    if (tile_gap) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->tiles[GRID_SIZE - 1][r]) {
                State_translate(state, NORTHWEST);
                return State_normalize(state);
            }
        }
        for (int q = 0; q < GRID_SIZE; q++) {
            if (state->tiles[q][GRID_SIZE - 1]) {
                State_translate(state, NORTHEAST);
                return State_normalize(state);
            }
        }
    }
}

void State_print(const struct State* s, FILE* stream)
{
    struct State state = *s;
    State_normalize(&state);

    // Convert to double-height coordinate space; see
    // https://www.redblobgames.com/grids/hexagons/#coordinates-doubled
    bool tiles[GRID_SIZE][GRID_SIZE * 3];
    uint8_t stacks[GRID_SIZE][GRID_SIZE * 3];
    memset(tiles, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE * 3);
    memset(stacks, 0, sizeof(uint8_t) * GRID_SIZE * GRID_SIZE * 3);

    int min_x = GRID_SIZE;
    int max_x = 0;
    int min_y = GRID_SIZE;
    int max_y = 0;
    for (int q = 0; q < GRID_SIZE - 1; q++) {
        for (int r = 0; r < GRID_SIZE - 1; r++) {
            // https://www.redblobgames.com/grids/hexagons/#conversions-doubled
            int x = q;
            int y = 2 * r + q;

            tiles[x][y] = state.tiles[q][r];
            stacks[x][y] = state.stacks[q][r];

            if (!tiles[x][y])
                continue;
            if (x < min_x)
                min_x = x;
            if (x > max_x)
                max_x = x;
            if (y < min_y)
                min_y = y;
            if (y > max_y)
                max_y = y;
        }
    }

    // If an odd column is the highest, add an undraw row above it (that
    // would have an even row as the highest). This is because the
    // following code assumes hexes in odd columns will always have a
    // hex to the northwest.
    if (min_y % 2 == 1) {
        min_y -= 1;
    }

    for (int x = min_x; x <= max_x + 1; x += 2) {
        bool here = tiles[x][min_y];
        fputc(' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(here ? '_' : ' ', stream);
        fputc(' ', stream);
    }
    fputc('\n', stream);
    for (int y = min_y; y <= max_y + 1; y += 2) {
        // Hexes take up two terminal lines
        // Line 1
        for (int x = min_x; x <= max_x + 1; x += 2) {
            bool here = tiles[x][y];
            bool nw = (x > 0 && y > 0) ? tiles[x - 1][y - 1] : false;
            bool ne = (x < max_x && y > 0) ? tiles[x + 1][y - 1] : false;
            bool se = x < max_x && y < max_y && tiles[x + 1][y + 1];

            fputc(here || nw ? '/' : ' ', stream);
            // TODO print stacks here, instead of placeholder x
            fputc(here ? 'x' : ' ', stream);
            fputc(here ? 'x' : ' ', stream);
            fputc(here || ne ? '\\' : ' ', stream);
            fputc(ne || se ? '_' : ' ', stream);
            fputc(ne || se ? '_' : ' ', stream);
        }
        fputc('\n', stream);
        // Line 2
        for (int x = min_x; x <= max_x + 1; x += 2) {
            bool here = tiles[x][y];
            bool sw = x > 0 && y < max_y && tiles[x - 1][y + 1];
            bool s = y + 2 <= max_y && tiles[x][y + 2];
            bool se = (x < max_x && y < max_y) ? tiles[x + 1][y + 1] : false;

            fputc(here || sw ? '\\' : ' ', stream);
            fputc(here || s ? '_' : ' ', stream);
            fputc(here || s ? '_' : ' ', stream);
            fputc(here || se ? '/' : ' ', stream);
            // TODO print stacks here, instead of placeholder x
            fputc(se ? 'x' : ' ', stream);
            fputc(se ? 'x' : ' ', stream);
        }
        fputc('\n', stream);
    }
}
