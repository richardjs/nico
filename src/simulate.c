#include <stdlib.h>

#include "mcts.h"
#include "state.h"

#ifdef WATCH_SIMS
#include "stateio.h"
#include <stdio.h>
#endif

/**
 * simulates play (in place) on a state, stopping at game end or
 * MAX_SIM_DEPTH, and returns 1.0 if the initial turn won, -1.0 if it
 * lost, and 0.0 on a draw or depth out
 */
float State_simulate(struct State* state,
    const struct MCTSOptions* options, struct MCTSStats* stats)
{
    stats->simulations++;

    enum Player original_turn = state->turn;

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    int depth = 0;
    while (actionc) {
        fprintf(stderr, "%d\n", actionc);
        struct Action* action = &actions[rand() % actionc];

#ifdef WATCH_SIMS
        Action_print(action, stderr);
#endif

        State_act(state, action);

        actionc = State_actions(state, actions);

#ifdef WATCH_SIMS
        State_print(state, stderr);
        char state_string[STATE_STRING_SIZE];
        State_to_string(state, state_string);
        printf("%s\n", state_string);
        getchar();
#endif
    }

    stats->mean_sim_depth += (depth - stats->mean_sim_depth) / stats->simulations;

    enum Player winner = State_winner(state);

    if (winner == DRAW) {
        return 0.0;
    }

    if (winner == original_turn) {
        return 1.0;
    }

    return -1.0;
}
