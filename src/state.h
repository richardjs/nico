#ifndef STATE_H
#define STATE_H

#include "coords.h"
#include <stdbool.h>
#include <stdint.h>

#define NUM_PLAYERS 2

#define PLAYER_TILES 4

#define MAX_TILE_HEXES (NUM_PLAYERS * PLAYER_TILES * 4)
#define MAX_PLACE_HEXES

#define INITIAL_STACK 16

// An active stack is one that has 2 or more tokens
#define MAX_ACTIVE_STACKS 8

#define MAX_ACTIONS 630

enum Player {
    P1 = 0,
    P2
};

struct Action {
    struct Coords start;
    struct Coords end;
    uint8_t count;

    // In place actions:
    //   - start is the existing hex the tile is being placed against
    //   - count is an enum TileDirection for the direction the
};

struct State {
    // TODO Break out separate TileState that can be shared among states
    // (since it never changes once the place phase is over)

    // Core information
    bool tiles[GRID_SIZE][GRID_SIZE];
    uint8_t stacks[GRID_SIZE][GRID_SIZE];

    uint8_t remaining_tiles[NUM_PLAYERS];

    struct Coords active_stacks[NUM_PLAYERS][MAX_ACTIVE_STACKS];
    uint8_t active_stackc[NUM_PLAYERS];

    enum Player turn;

    // Derived information
    struct Coords tile_hexes[MAX_TILE_HEXES];
    uint8_t tile_hexc;
};

void State_new(struct State* state);
int State_actions(const struct State* state, struct Action actions[]);
void State_act(struct State* state, const struct Action* action);

#endif
