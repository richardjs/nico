#include "state.h"
#include "tile.h"
#include <string.h>

void State_new(struct State* state, struct TileState* tile_state)
{
    memset(state, 0, sizeof(struct State));
    memset(tile_state, 0, sizeof(struct TileState));

    state->tile_state = tile_state;

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
        actions[c].end.q = TILE_END_SOUTHEAST;
        actions[c++].count = 0;

        actions[c].start.q = 0;
        actions[c].start.r = 0;
        actions[c].end.q = TILE_END_EAST;
        actions[c++].count = 0;

        actions[c].start.q = 0;
        actions[c].start.r = 0;
        actions[c].end.q = TILE_END_NORTHEAST;
        actions[c++].count = 0;
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
            if (!state->tile_state->tiles[tile.origin.q][tile.origin.r]) {
                // and if so, using that as the tile origin, for each tile direction,
                for (int td = 0; td < NUM_TILE_DIRECTIONS; td++) {
                    tile.direction = td;
                    Tile_coords(&tile, place_coords);
                    // see if that tile will fit.
                    // (start at 1 because we've already checked the origin)
                    bool tile_clear = true;
                    for (int j = 1; j < TILE_SIZE && tile_clear; j++) {
                        tile_clear = !state->tile_state->tiles[place_coords[j].q][place_coords[j].r];
                    }

                    // If it it does, create a place action there.
                    // TODO Check for existing duplicate places?
                    if (tile_clear) {
                        actions[c].start = tile.origin;
                        actions[c].end.q = tile.direction;
                        actions[c++].count = 0;
                    }
                }
            }
        }
    }

    return c;
}

int flood_fill(const bool tiles[][GRID_SIZE], const struct Coords* start, bool perimeter[][GRID_SIZE])
{
    memset(perimeter, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);

    struct Coords stack[GRID_SIZE * GRID_SIZE];
    int stackc = 0;
    stack[stackc++] = *start;

    bool crumbs[GRID_SIZE][GRID_SIZE] = { false };
    crumbs[start->q][start->r] = true;

    int walked = 0;

    struct Coords walk;

    while (stackc > 0) {
        struct Coords pos = stack[--stackc];

        walked++;

        for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
            walk = pos;
            Coords_move(&walk, d);
            if (tiles[walk.q][walk.r]) {
                perimeter[walk.q][walk.r] = true;
                continue;
            }
            if (crumbs[walk.q][walk.r]) {
                continue;
            }

            stack[stackc++] = walk;
            crumbs[walk.q][walk.r] = true;
        }
    }

    return walked;
}

void State_find_perimeter(struct State* state, bool perimeter[][GRID_SIZE])
{
    struct Coords start;
    for (start.q = 0; start.q < GRID_SIZE; start.q++) {
        for (start.r = 0; start.r < GRID_SIZE; start.r++) {
            if (state->tile_state->tiles[start.q][start.r]) {
                continue;
            }
            memset(perimeter, 0, sizeof(bool) * GRID_SIZE * GRID_SIZE);
            int count = flood_fill(state->tile_state->tiles, &start, perimeter);
            if (count > MAX_HOLE_SIZE) {
                return;
            }
        }
    }
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
        // TODO we could store this in tile_state instead of calculating it twice
        // otoh, we only do it twice per game
        bool perimeter[GRID_SIZE][GRID_SIZE];
        State_find_perimeter(state, perimeter);

        for (int i = 0; i < state->tile_hexc; i++) {
            const struct Coords* hex = &state->tile_hexes[i];
            if (!perimeter[hex->q][hex->r]) {
                continue;
            }
            if (state->stacks[hex->q][hex->r]) {
                continue;
            }
            actions[c].start = *hex;
            actions[c++].count = INITIAL_STACK;
        }
        return c;
    }

    // Stack moves
    for (int i = 0; i < state->player_stackc[state->turn]; i++) {
        const struct Coords* start = &state->player_stacks[state->turn][i];

        if (state->stacks[start->q][start->r] == 1) {
            continue;
        }

        for (enum Direction d = 0; d < NUM_DIRECTIONS; d++) {
            // TODO may be able to optimize here
            struct Coords end = *start;
            struct Coords walk = *start;

            Coords_move(&walk, d);
            while (state->tile_state->tiles[walk.q][walk.r] && state->stacks[walk.q][walk.r] == 0) {
                end = walk;
                Coords_move(&walk, d);
            }

            if (start->q == end.q && start->r == end.r) {
                continue;
            }

            for (int n = 1; n < state->stacks[start->q][start->r]; n++) {
                actions[c].start = *start;
                actions[c].end = end;
                actions[c++].count = n;
            }
        }
    }

    return c;
}

void State_place_act(struct State* state, const struct Action* action)
{
    struct Coords place_coords[TILE_SIZE];
    struct Tile tile = {
        .origin = action->start,
        .direction = action->end.q,
    };
    Tile_coords(&tile, place_coords);

    for (int i = 0; i < TILE_SIZE; i++) {
        state->tile_state->tiles[place_coords[i].q][place_coords[i].r] = true;
        state->tile_hexes[state->tile_hexc++] = place_coords[i];
    }

    state->remaining_tiles[state->turn]--;
}

// Helper function to reate a new stack of a given size at a given hex,
// while also remembering to update player_stacks
void State_new_stack_hex(struct State* state, const struct Coords* coords, uint8_t count)
{
    state->stacks[coords->q][coords->r] = count;
    state->player_stacks[state->turn][state->player_stackc[state->turn]++] = *coords;
}

void State_act(struct State* state, const struct Action* action)
{
    // Skip the turn, e.g. when player has no actions
    if (action == NULL) {
        goto next_turn;
    }

    // Tile place
    if (state->remaining_tiles[state->turn] > 0) {
        State_place_act(state, action);
        goto next_turn;
    }

    // Initial stack place
    if (action->count == INITIAL_STACK) {
        State_new_stack_hex(state, &action->start, INITIAL_STACK);
        goto next_turn;
    }

    // Stack move
    State_new_stack_hex(state, &action->end, action->count);
    state->stacks[action->start.q][action->start.r] -= action->count;

next_turn:
    state->turn = !state->turn;
}
