#include "stateutil.h"
#include "state.h"

void State_derive(struct State* state)
{
    state->tile_hexc = 0;
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->tiles[q][r]) {
                state->tile_hexes[state->tile_hexc].q = q;
                state->tile_hexes[state->tile_hexc++].r = r;
            }
        }
    }
}

unsigned int State_compare(const struct State* s1, const struct State* s2)
{
    // .turn
    if (s1->turn != s2->turn) {
        return 1;
    }

    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            // .tiles
            if (s1->tiles[q][r] != s2->tiles[q][r]) {
                return 2;
            }
            // .stacks
            if (s1->stacks[q][r] != s2->stacks[q][r]) {
                return 3;
            }
        }
    }

    for (int p = 0; p < NUM_PLAYERS; p++) {
        // .remaining_tiles
        if (s1->remaining_tiles[p] != s2->remaining_tiles[p]) {
            return 4;
        }
        // .active_stackc
        if (s1->active_stackc[p] != s2->active_stackc[p]) {
            return 5;
        }
        // .active_stacks
        for (int i = 0; i < s1->active_stackc[p]; i++) {
            if (s1->active_stacks[p][i].q != s2->active_stacks[p][i].q) {
                return 6;
            }
            if (s1->active_stacks[p][i].r != s2->active_stacks[p][i].r) {
                return 7;
            }
        }
    }

    // .tile_hexc
    if (s1->tile_hexc != s2->tile_hexc) {
        return 8;
    }
    // .tile_hexes
    for (int i = 0; i < s1->tile_hexc; i++) {
        if (s1->tile_hexes[i].q != s2->tile_hexes[i].q) {
            return 9;
        }
        if (s1->tile_hexes[i].r != s2->tile_hexes[i].r) {
            printf("%d %d\n", s1->tile_hexes[i].r, s2->tile_hexes[i].r);
            printf("%d %d\n", i, s1->tile_hexc);
            return 10;
        }
    }

    return 0;
}
