#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "mcts.h"
#include "state.h"
#include "stateio.h"
#include "think.h"

#define TOP_ACTIONS 10

void think(
    const struct State* state,
    struct MCTSResults* results,
    const struct MCTSOptions* options,
    int workers)
{
    char state_string[STATE_STRING_SIZE];
    char action_string[ACTION_STRING_SIZE];

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    if (actionc == 1) {
        fprintf(stderr, "Single action\n");

        results->presearch_action = &actions[0];

        struct State after;
        struct TileState after_tiles;
        State_copy(state, &after, &after_tiles);
        State_act(&after, &actions[0]);
        State_normalize(&after);
        State_to_string(&after, state_string);
        fprintf(stderr, "next:\t%s\n", state_string);
        return;
    }

    fprintf(stderr, "MCTS options:\titerations=%ld seconds=%ld workers=%d uctc=%.2f\n",
        options->iterations,
        options->seconds,
        workers,
        options->uctc);

    int pipefd[2];
    pipe(pipefd);

    for (int i = 0; i < workers; i++) {
        srand(rand());
        if (fork() > 0) {
            continue;
        }
        struct MCTSResults results;
        mcts(state, &results, options);
        write(pipefd[1], &results, sizeof(struct MCTSResults));
        exit(0);
    }

    memset(results, 0, sizeof(struct MCTSResults));

    // TODO
    struct timeval start;
    gettimeofday(&start, NULL);

    for (int i = 0; i < workers; i++) {
        wait(NULL);

        struct MCTSResults worker_results;
        read(pipefd[0], &worker_results, sizeof(struct MCTSResults));

        for (int j = 0; j < actionc; j++) {
            results->nodes[j].visits += worker_results.nodes[j].visits;
            results->nodes[j].value += worker_results.nodes[j].value;
        }

        results->stats.iterations += worker_results.stats.iterations;
        results->stats.nodes += worker_results.stats.nodes;
        results->stats.tree_bytes += worker_results.stats.tree_bytes;
        results->stats.simulations += worker_results.stats.simulations;
        results->stats.early_terminations += worker_results.stats.early_terminations;
        results->stats.mean_sim_depth += worker_results.stats.mean_sim_depth / workers;
        results->stats.change_iterations = results->stats.change_iterations > worker_results.stats.change_iterations ? results->stats.change_iterations : worker_results.stats.change_iterations;
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    results->stats.duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    results->score = -INFINITY;
    int top_actionis[TOP_ACTIONS];
    memset(top_actionis, -1, sizeof(int) * TOP_ACTIONS);
    for (int i = 0; i < actionc; i++) {
        float score = -1 * results->nodes[i].value / results->nodes[i].visits;
        printf("%f\n", score);

        if (score >= results->score) {
            results->score = score;
            results->actioni = i;
        }

        for (int j = 0; j < TOP_ACTIONS && j < actionc; j++) {
            if (top_actionis[j] < 0) {
                top_actionis[j] = i;
                break;
            }

            float s = -1 * results->nodes[top_actionis[j]].value / results->nodes[top_actionis[j]].visits;
            if (score > s) {
                for (int k = TOP_ACTIONS - 2; k >= j; k--) {
                    top_actionis[k + 1] = top_actionis[k];
                }
                top_actionis[j] = i;
                break;
            }
        }
    }

    struct State after;
    struct TileState after_tiles;
    State_copy(state, &after, &after_tiles);
    State_act(&after, &actions[results->actioni]);

    fprintf(stderr, "score:\t\t%.2f\n", results->score);

    fprintf(stderr, "iterations:\t%ld\n", results->stats.iterations);
    fprintf(stderr, "change iters:\t%d\n", results->stats.change_iterations);
    fprintf(stderr, "time:\t\t%ld ms\n", results->stats.duration);
    fprintf(stderr,
        "iters/s:\t%ld\n",
        results->stats.duration
            ? 1000 * results->stats.iterations / results->stats.duration
            : 0);
    fprintf(stderr, "actions:\t%d\n", actionc);
    fprintf(stderr, "action iters:\t%d\n", results->nodes[results->actioni].visits);
    fprintf(stderr, "mean sim depth:\t%.2f\n", results->stats.mean_sim_depth);
    fprintf(stderr, "simulations:\t%d\n", results->stats.simulations);
    fprintf(stderr, "early terms:\t%ld\n", results->stats.early_terminations);
    fprintf(
        stderr, "tree size:\t%ld MiB\n", results->stats.tree_bytes / 1024 / 1024);

    for (int i = 0; i < TOP_ACTIONS && i < actionc; i++) {
        Action_to_string(&actions[top_actionis[i]], action_string);
        float score = -1 * results->nodes[top_actionis[i]].value / results->nodes[top_actionis[i]].visits;
        fprintf(stderr, "%.2f\t%s\t%d\n", score, action_string, results->nodes[top_actionis[i]].visits);
    }

    State_normalize(&after);
    State_to_string(&after, state_string);
    fprintf(stderr, "next:\t%s\n", state_string);
}
