#ifndef STATE_H
#define STATE_H

#include "coords.h"
#include <stdbool.h>
#include <stdint.h>

#define NUM_PLAYERS 2

#define PLAYER_TILES 4

#define MAX_TILE_HEXES (NUM_PLAYERS * PLAYER_TILES * 4)

#define INITIAL_STACK 16

#define MAX_ACTIONS 630

// How many hexes can a hole in the board contain; used in perimeter detection
#define MAX_HOLE_SIZE 24

#define PASS_ACTION (INITIAL_STACK + 1)

// Regions are discrete empty areas of the board, divided by stacks
// TODO We can probably prove a lower number than this
#define MAX_REGIONS 16
#define NO_REGION (MAX_REGIONS + 1)

enum Player {
    P1 = 0,
    P2,
    DRAW,
    NO_WINNER
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

    // If count == PASS_ACTION, player has no moves
};

struct TileState {
    bool tiles[GRID_SIZE][GRID_SIZE];
};

struct State {
    // Core information

    struct TileState* tile_state;
    uint8_t stacks[GRID_SIZE][GRID_SIZE];

    uint8_t remaining_tiles[NUM_PLAYERS];

    struct Coords player_stacks[NUM_PLAYERS][INITIAL_STACK];
    uint8_t player_stackc[NUM_PLAYERS];

    enum Player turn;

    // Derived information

    // List of hexes that have a tile
    struct Coords tile_hexes[MAX_TILE_HEXES];
    uint8_t tile_hexc;

    uint8_t regions[GRID_SIZE][GRID_SIZE];
    uint8_t region_size[MAX_REGIONS];
    // Number each player has available to move into the region'
    uint8_t region_available[MAX_REGIONS][NUM_PLAYERS];
    // Same as above, but only using a single stack (the greatest)
    uint8_t region_single_available[MAX_REGIONS][NUM_PLAYERS];
    uint8_t regionc;

    // Sum of [min(stack size - 1, adjacent region area) for all player stacks]
    // Quick to calculate, and can be used for early termination
    uint8_t quick_usable[NUM_PLAYERS];
};

void State_new(struct State* state, struct TileState* tile_state);

int State_actions(const struct State* state, struct Action actions[]);
void State_act(struct State* state, const struct Action* action);

void State_copy(const struct State* src, struct State* state, struct TileState* tile_state);

// IMPORTANT: This assumes a terminal state, i.e. no actions
enum Player State_winner(const struct State* state);

#endif
