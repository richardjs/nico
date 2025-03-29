#include "stateutil.h"
#include "coords.h"
#include "errorcodes.h"
#include "state.h"
#include <stdlib.h>

// TODO tmp
#include "stateio.h"

uint8_t region_flood_fill(
    const struct State* state, const struct Coords* start,
    struct Coords hexes[MAX_TILE_HEXES], uint8_t available[NUM_PLAYERS]);

void State_derive(struct State* state)
{
    state->tile_hexc = 0;
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state->tile_state->tiles[q][r]) {
                state->tile_hexes[state->tile_hexc].q = q;
                state->tile_hexes[state->tile_hexc++].r = r;
            }
        }
    }

    state->regionc = 0;

    // Set all hexes to MAX_REGIONS + 1 so we can track where we've calculated
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            state->regions[q][r] = MAX_REGIONS + 1;
        }
    }

    struct Coords start;
    struct Coords hexes[MAX_TILE_HEXES];
    uint8_t available[NUM_PLAYERS];
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (!state->tile_state->tiles[q][r]) {
                continue;
            }
            if (state->stacks[q][r]) {
                continue;
            }
            if (state->regions[q][r] != MAX_REGIONS + 1) {
                continue;
            }

            start.q = q;
            start.r = r;
            int hexc = region_flood_fill(state, &start, hexes, available);

            uint8_t region = state->regionc++;
            state->region_size[region] = hexc;
            for (int i = 0; i < hexc; i++) {
                state->regions[hexes[i].q][hexes[i].r] = region;
            }
            state->region_available[region][P1] = available[P1] < hexc ? available[P1] : hexc;
            state->region_available[region][P2] = available[P2] < hexc ? available[P2] : hexc;
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
            if (s1->tile_state->tiles[q][r] != s2->tile_state->tiles[q][r]) {
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
        // .player_stackc
        if (s1->player_stackc[p] != s2->player_stackc[p]) {
            return 5;
        }
        // .player_stacks
        // These can be in arbitrary order, so we need to look for a s2 match for each one
        for (int i = 0; i < s1->player_stackc[p]; i++) {
            bool found_match = false;
            for (int j = 0; j < s1->player_stackc[p]; j++) {
                if (s1->player_stacks[p][i].q == s2->player_stacks[p][j].q
                    && s1->player_stacks[p][i].r == s2->player_stacks[p][j].r) {
                    found_match = true;
                    break;
                }
            }
            if (!found_match) {
                return 6;
            }
        }
    }

    // .tile_hexc
    if (s1->tile_hexc != s2->tile_hexc) {
        return 7;
    }
    // .tile_hexes
    // These can be in arbitrary order, so we need to look for a s2 match for each one
    for (int i = 0; i < s1->tile_hexc; i++) {
        bool found_match = false;
        for (int j = 0; j < s1->tile_hexc; j++) {
            if (s1->tile_hexes[i].q == s2->tile_hexes[j].q
                && s1->tile_hexes[i].r == s2->tile_hexes[j].r) {
                found_match = true;
                break;
            }
        }
        if (!found_match) {
            return 8;
        }
    }

    // .regionc
    if (s1->regionc != s2->regionc) {
        return 9;
    }

    // These can be in arbitrary order, so we need to look for a s2 match for each one
    for (int i = 0; i < s1->regionc; i++) {
        bool found_match = false;
        for (int j = 0; j < s1->regionc; j++) {
            // .region_size
            if (s1->region_size[i] != s2->region_size[j]) {
                continue;
            }
            // .region_available
            if (s1->region_available[i][P1] != s2->region_available[i][P1]
                && s1->region_available[i][P2] != s2->region_available[i][P2]) {
                continue;
            }

            // .region
            bool same_hexes = true;
            for (int q = 0; q < GRID_SIZE; q++) {
                for (int r = 0; r < GRID_SIZE; r++) {
                    if (s1->regions[q][r] == i) {
                        if (s2->regions[q][r] != j) {
                            same_hexes = false;
                            break;
                        }
                    }
                }
            }
            if (!same_hexes) {
                continue;
            }

            found_match = true;
            break;
        }

        if (!found_match) {
            return 10;
        }
    }

    return 0;
}

bool Action_compare(const struct Action* a1, const struct Action* a2)
{
    return a1->start.q != a2->start.q
        || a1->start.r != a2->start.r
        || a1->count != a2->count
        || a1->end.q != a2->end.q
        || a1->end.r != a2->end.r;
}

enum Player State_stack_player(const struct State* state, const struct Coords* coords)
{
    for (enum Player p = 0; p < NUM_PLAYERS; p++) {
        for (int i = 0; i < state->player_stackc[p]; i++) {
            const struct Coords* hex = &state->player_stacks[p][i];
            if (hex->q == coords->q && hex->r == coords->r) {
                return p;
            }
        }
    }

    // Error case
    exit(ERROR_NO_STACK_PLAYER);
    return -1;
}

bool State_terminal(const struct State* s)
{
    struct Action actions[MAX_ACTIONS];
    if (State_actions(s, actions) != 1 && actions[0].count != PASS_ACTION) {
        return false;
    }

    struct State state;
    struct TileState tile_state;
    State_copy(s, &state, &tile_state);
    State_act(&state, &actions[0]);

    return State_actions(&state, actions) == 1 && actions[0].count == PASS_ACTION;
}

uint8_t State_best_uncontested_region(const struct State* state)
{
    uint8_t best_uncontested_region = 0;
    for (int i = 0; i < state->regionc; i++) {
        if (state->region_available[i][!state->turn] > 0) {
            continue;
        }

        if (state->region_available[i][state->turn]
            >= state->region_available[best_uncontested_region][state->turn]) {
            best_uncontested_region = i;
        }
    }
    return best_uncontested_region;
}

// TODO tmp
void State_print_regions(struct State* state);

int region_fill_dfs(const struct State* state)
{
    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    if (actions[0].count == PASS_ACTION) {
        for (int q = 0; q < GRID_SIZE; q++) {
            for (int r = 0; r < GRID_SIZE; r++) {
                if (!state->stacks[q][r]) {
                    return -1;
                }
            }
        }
        State_print(state, stdout);
        return 0;
    }

    for (int i = actionc - 1; i >= 0; i--) {
        struct State after;
        struct TileState after_tiles;
        State_copy(state, &after, &after_tiles);
        State_act(&after, &actions[i]);

        // Keep it the original player's turn
        after.turn = state->turn;

        if (region_fill_dfs(&after) >= 0) {
            return i;
        }
    }

    return -1;
}

void State_fill_uncontested_region_action(const struct State* s, struct Action* action)
{
    struct State state;
    struct TileState tile_state;
    State_copy(s, &state, &tile_state);

    uint8_t region = State_best_uncontested_region(&state);

    // Remove all hexes except those in the region (and those under stacks)
    for (int q = 0; q < GRID_SIZE; q++) {
        for (int r = 0; r < GRID_SIZE; r++) {
            if (state.regions[q][r] == region || state.stacks[q][r]) {
                continue;
            }
            state.tile_state->tiles[q][r] = false;
        }
    }

    // We changed core information, so derive
    State_derive(&state);

    State_print(&state, stdout);

    // DFS to fill up region
    struct State state_stack[INITIAL_STACK];
    struct TileState tilestate_stack[INITIAL_STACK];
    struct Action actions_stack[INITIAL_STACK];
    int sp = 0;

    State_copy(&state, &state_stack[sp], &tilestate_stack[sp]);
    region_fill_dfs(&state);
}
