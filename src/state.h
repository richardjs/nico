#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include <stdint.h>

#define NUM_PLAYERS 2

#define PLAYER_TILES 4

#define INITIAL_STACK 16

// An active stack is one that has 2 or more tokens
#define MAX_ACTIVE_STACKS 8

#define GRID_SIZE 24

#define MAX_ACTIONS 630

enum Player {
    P1 = 0,
    P2
}

/*
    /1,0\       /0,0\           /1,0\
/0,1\___/2,0\   \___/1,0\   /0,1\___/
\___/1,1\___/   /0,1\___/   \___/1,1\
    \___/       \___/1,1\   /0,2\___/
                    \___/   \___/
Type H^         Type L^     Type R^
*/

enum Rotation {
    H = 0,
    L,
    R
}

struct Action {
    struct Coords start;
    struct Coords end;
    uint8_t count;

    // In place actions:
    //   - start is the existing space the tile is being placed against
    //   - count is an enum TileDirection for the direction the
};

struct State {
    bool tiles[GRID_SIZE][GRID_SIZE];
    uint8_t stacks[GRID_SIZE][GRID_SIZE];

    uint8_t remaining_tiles[NUM_PLAYERS];

    struct Coords active_stacks[NUM_PLAYERS][MAX_ACTIVE_STACKS];

    enum Player turn;
};

void State_new(struct State* state);

#endif
