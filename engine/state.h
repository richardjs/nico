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

#define MAX_ACTIONS 630

// How many hexes can a hole in the board contain; used in perimeter detection
#define MAX_HOLE_SIZE 24

enum Player {
    P1 = 0,
    P2,
    DRAW
};

struct Action {
    struct Coords start;
    struct Coords end;
    uint8_t count;

    // In tile place actions:
    //   - start is the origin point of the tile
    //   - end.q is an enum TileDirection for the direction the
    //   - count is 0

    // In initial stack place actions:
    //   - start is the hex the stack is placed on
    //   - count is INITIAL_STACK (16)
};

struct TileState {
    bool tiles[GRID_SIZE][GRID_SIZE];
};

struct State {
    // TODO Break out separate TileState that can be shared among states
    // (since it never changes once the place phase is over)

    // Core information
    struct TileState* tile_state;
    uint8_t stacks[GRID_SIZE][GRID_SIZE];

    uint8_t remaining_tiles[NUM_PLAYERS];

    struct Coords player_stacks[NUM_PLAYERS][INITIAL_STACK];
    uint8_t player_stackc[NUM_PLAYERS];

    enum Player turn;

    // Derived information
    struct Coords tile_hexes[MAX_TILE_HEXES];
    uint8_t tile_hexc;
};

void State_new(struct State* state, struct TileState* tile_state);

int State_actions(const struct State* state, struct Action actions[]);
void State_act(struct State* state, const struct Action* action);

void State_copy(const struct State* src, struct State* state, struct TileState* tile_state);

// IMPORTANT: This assumes a terminal state, i.e. no actions
enum Player State_winner(const struct State* state);

#endif
