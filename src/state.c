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
        actions[c].start.q = 0;
        actions[c].start.r = 0;
        actions[c++].count = TILE_SOUTHEAST;

        actions[c].start.q = 0;
        actions[c].start.r = 1;
        actions[c++].count = TILE_EAST;

        actions[c].start.q = 0;
        actions[c].start.r = 2;
        actions[c++].count = TILE_NORTHEAST;
        return c;
    }

    // TODO
    return c;
}

int State_actions(const struct State* state, struct Action actions[])
{
    if (state->remaining_tiles[state->turn] > 0) {
        return State_place_actions(state, actions);
    }

    // TODO
    return 0;
}

void State_place_act(struct State* state, const struct Action* action)
{
    struct Coords place_coords[TILE_SIZE];
    tile_coords(&action->start, action->count, place_coords);

    for (int i = 0; i < TILE_SIZE; i++) {
        state->tiles[place_coords[i].q][place_coords[i].r] = true;
    }

    state->remaining_tiles[state->turn]--;
    state->turn = !state->turn;
}

void State_act(struct State* state, const struct Action* action)
{
    if (state->remaining_tiles[state->turn] > 0) {
        return State_place_act(state, action);
    }
}
