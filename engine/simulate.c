#include <stdlib.h>

#include "mcts.h"
#include "state.h"

#ifdef WATCH_SIMS
#include "stateio.h"
#include <stdio.h>
#endif

// TODO tmp
#include "stateio.h"

enum Player State_early_winner(const struct State* state)
{
    // Don't check in the placement phases
    if (state->player_stackc[state->turn] == 0) {
        return NO_WINNER;
    }

    uint8_t p1_score = state->player_stackc[P1];
    uint8_t p2_score = state->player_stackc[P2];

    uint8_t p1_available;
    uint8_t p2_available;
    uint8_t p1_best_uncontested = 0;
    uint8_t p2_best_uncontested = 0;
    uint8_t p1_total_available = 0;
    uint8_t p2_total_available = 0;
    for (int i = 0; i < state->regionc; i++) {
        p1_available = state->region_available[i][P1];
        p2_available = state->region_available[i][P2];

        if (p2_available == 0 && p1_available > p1_best_uncontested) {
            p1_best_uncontested = p1_available;
        } else if (p1_available == 0 && p2_available > p2_best_uncontested) {
            p2_best_uncontested = p2_available;
        }

        p1_total_available += p1_available;
        p2_total_available += p2_available;

        // printf("region %d size %d available: %d %d\n", i, state->region_size[i], p1_available, p2_available);
    }

    // printf("%d+%d (%d) %d+%d (%d)\n", p1_score, p1_best_uncontested, p1_total_available, p2_score, p2_best_uncontested, p2_total_available);
    // State_print(state, stdout);

    if (p1_score + p1_best_uncontested > p2_score + p2_total_available) {
        return P1;
    } else if (p2_score + p2_best_uncontested > p1_score + p1_total_available) {
        return P2;
    }
    // TODO we can probably refine this
    else if (p1_score + p1_best_uncontested == 16 && p2_score + p2_best_uncontested == 16) {
        return DRAW;
    }

    return NO_WINNER;
}

float State_simulate(struct State* state,
    const struct MCTSOptions* options, struct MCTSStats* stats)
{
    stats->simulations++;

    enum Player original_turn = state->turn;

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    int depth = 0;
    bool other_player_passed = false;
    enum Player winner;
    while (1) {
        struct Action* action = &actions[rand() % actionc];

        if (actionc == 1 && action->count == PASS_ACTION) {
            // Terminal statea
            if (other_player_passed) {
                break;
            }
            other_player_passed = true;
        } else {
            other_player_passed = true;
        }

#ifdef WATCH_SIMS
        Action_print(action, stderr);
#endif

        State_act(state, action);

#ifdef WATCH_SIMS
        State_print(state, stderr);
        char state_string[STATE_STRING_SIZE];
        State_to_string(state, state_string);
        printf("%s\n", state_string);
        getchar();
#endif

        enum Player early_winner = State_early_winner(state);
        if (early_winner != NO_WINNER) {
            winner = early_winner;
            stats->early_terminations++;
            goto have_winner;
        }

        actionc = State_actions(state, actions);
    }

    winner = State_winner(state);

have_winner:

    stats->mean_sim_depth += (depth - stats->mean_sim_depth) / stats->simulations;

    if (winner == DRAW) {
        return 0.0;
    }

    if (winner == original_turn) {
        return 1.0;
    }

    return -1.0;
}
