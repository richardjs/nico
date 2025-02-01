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

    // Special case for first action of game
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

    struct Tile tile;
    struct Coords place_coords[TILE_SIZE];
    // For each tile hex,
    for (int i = 0; i < state->tile_hexc; i++) {
        // and each direction,
        for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
            // check if the hex in that direction is empty,
            tile.origin = state->tile_hexes[i];
            Coords_move(&tile.origin, d);
            if (!state->tiles[tile.origin.q][tile.origin.r]) {
                // and if so, using that as the tile origin, for each tile direction,
                for (int td = 0; td < NUM_DIRECTIONS; td++) {
                    tile.direction = td;
                    Tile_coords(&tile, place_coords);
                    // see if that tile will fit.
                    // (start at 1 because we've already checked the origin)
                    bool tile_clear = true;
                    for (int j = 1; j < TILE_SIZE && tile_clear; j++) {
                        tile_clear = !state->tiles[place_coords[j].q][place_coords[j].r];
                    }

                    // If it it does, create a place action there.
                    // TODO Check for existing duplicate places?
                    if (tile_clear) {
                        actions[c].start = tile.origin;
                        actions[c++].count = tile.direction;
                    }
                }
            }
        }
    }

    return c;
}

int State_actions(const struct State* state, struct Action actions[])
{
    // Tile placement phase
    if (state->remaining_tiles[state->turn] > 0) {
        return State_place_actions(state, actions);
    }

    int c = 0;

    // Initial stack placement
    if (state->player_stackc[state->turn] == 0) {
        for (int i = 0; i < state->tile_hexc; i++) {
            const struct Coords* hex = &state->tile_hexes[i];
            if (state->stacks[hex->q][hex->r]) {
                continue;
            }
            actions[c].start = *hex;
            actions[c++].count = INITIAL_STACK;
        }
    }

    // Stack moves
    // TODO
    return 0;
}

void State_place_act(struct State* state, const struct Action* action)
{
    struct Coords place_coords[TILE_SIZE];
    struct Tile tile = {
        .origin = action->start,
        .direction = action->count,
    };
    Tile_coords(&tile, place_coords);

    for (int i = 0; i < TILE_SIZE; i++) {
        state->tiles[place_coords[i].q][place_coords[i].r] = true;
        state->tile_hexes[state->tile_hexc++] = place_coords[i];
    }

    state->remaining_tiles[state->turn]--;
}

void State_new_stack_tile(struct State* state, const struct Coords* coords, uint8_t count)
{
    state->stacks[coords->q][coords->r] = count;
    state->player_stacks[state->turn][state->player_stackc[state->turn]++] = *coords;
}

void State_act(struct State* state, const struct Action* action)
{
    if (state->remaining_tiles[state->turn] > 0) {
        State_place_act(state, action);
        goto next_turn;
    }

    if (action->count == INITIAL_STACK) {
        State_new_stack_tile(state, &action->start, INITIAL_STACK);
        goto next_turn;
    }

next_turn:
    state->turn = !state->turn;
}
