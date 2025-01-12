#include "state.h"
#include "tile.h"
#include <string.h>

void State_new(struct State* state)
{
    memset(state, 0, sizeof(struct State));

    for (int i = 0; i < NUM_PLAYERS; i++) {
        state->remaining_tiles[i] = PLAYER_TILES;
    }
}

int State_place_actions(const struct State* state, struct Action actions[])
{
    int c = 0;

    // First action of game
    if (state->remaining_tiles[P1] == PLAYER_TILES) {
        actions[c].q = 0;
        actions[c].r = 0;
        actions[c++].count = TILE_SOUTHEAST;

        actions[c].q = 0;
        actions[c].r = 1;
        actions[c++].count = TILE_EAST;

        actions[c].q = 0;
        actions[c].r = 2;
        actions[c++].count = TILE_NORTHEAST;
        return c;
    }
}

int State_actions(const struct State* state, struct Action actions[])
{
    if (state->remaining_tiles[state->turn] > 0) {
        return State_place_actions(state, actions);
    }
}

void State_place_act(struct State* state, const struct Action* action)
{
    
}

void State_act(struct State* state, const struct Action* action)
{
    if (state->remaining_tiles[state->turn] > 0) {
        return State_place_act(state, action);
    }
}
