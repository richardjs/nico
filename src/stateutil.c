#include "stateutil.h"
#include "state.h"

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
}

void State_copy(const struct State* src, struct State* state, struct TileState* tile_state)
{
    *state = *src;
    *tile_state = *src->tile_state;
    state->tile_state = tile_state;
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
        for (int i = 0; i < s1->player_stackc[p]; i++) {
            if (s1->player_stacks[p][i].q != s2->player_stacks[p][i].q) {
                return 6;
            }
            if (s1->player_stacks[p][i].r != s2->player_stacks[p][i].r) {
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
    return -1;
}

bool State_terminal(const struct State* s)
{
    struct Action actions[MAX_ACTIONS];
    if (State_actions(s, actions) != 0) {
        return false;
    }

    struct State state;
    struct TileState tile_state;
    State_copy(s, &state, &tile_state);
    State_act(&state, NULL);

    return State_actions(&state, actions) == 0;
}
