#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "mcts.h"
#include "simulate.h"
#include "state.h"

// store these globally so we don't have to pass them around
static struct MCTSOptions options;
static struct MCTSResults* results;

/**
 * mallocs, checks for null, and increases results.stats.tree_bytes
 */
void* mctsmalloc(size_t size)
{
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "ERROR: failure to malloc in MCTS\n");
        exit(1);
    }
    results->stats.tree_bytes += size;
    return ptr;
}

void MCTSOptions_default(struct MCTSOptions* o)
{
    o->iterations = DEFAULT_ITERATIONS;
    o->uctc = DEFAULT_UCTC;
    o->max_sim_depth = DEFAULT_MAX_SIM_DEPTH;
    o->save_tree = DEFAULT_SAVE_TREE;
}

void Node_init(struct Node* node, uint8_t depth)
{
    node->expanded = false;
    node->visits = 0;
    node->value = 0;
    // TODO we probably could pass this around mcts() and iterate()
    // instead of storing it here
    node->depth = depth;

    results->stats.nodes++;
    if (depth > results->stats.tree_depth) {
        results->stats.tree_depth = depth;
    }
}

/**
 * allocates space for children pointers and the child nodes themselves,
 * and calls Node_init on each child
 */
void Node_expand(struct Node* node, const struct State* state, int actionc)
{
    node->children_count = actionc;
    node->children = mctsmalloc(sizeof(struct Node*) * node->children_count);

    // TODO
    for (int i = 0; i < actionc; i++) {
        node->children[i] = mctsmalloc(sizeof(struct Node));
        Node_init(node->children[i], node->depth + 1);
    }

    node->expanded = true;
}

/**
 * frees the node's children along with the node
 */
void Node_free(struct Node* node)
{
    if (node->expanded) {
        for (int i = 0; i < node->children_count; i++) {
            Node_free(node->children[i]);
        }
        free(node->children);
    }

    free(node);
}

/**
 * single MCTS iteration: recursively walk down tree with state
 * (choosing promising children), simulate when we get to the end of the
 * tree, and update visited nodes with the results
 */
float iterate(struct Node* root, struct State* state)
{
    // Terminal state
    if (root->children == 0) {
        root->visits++;

        enum Player winner = State_winner(state);

        float value;
        if (winner == DRAW) {
            value = 0;
            // TODO double-check that this all lines up
        } else if (winner == state->turn) {
            value = 1.0;
        } else {
            value = -1.0;
        }

        root->value += value;
        return value;
    }

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    if (!root->expanded) {
        Node_expand(root, state, actionc);
    }

    if (root->visits == 0) {
        float score = State_simulate(state, &options, &results->stats);

        root->visits++;
        root->value += score;
        return score;
    }

    int childi = 0;
    float best_uct = -INFINITY;
    for (int i = 0; i < actionc; i++) {
        if (root->children[i]->visits == 0) {
            childi = i;
            break;
        }

        float uct = -1 * root->children[i]->value / root->children[i]->visits + options.uctc * sqrtf(logf(root->visits) / root->children[i]->visits);

        if (uct >= best_uct) {
            best_uct = uct;
            childi = i;
        }
    }

    struct Node* child = root->children[childi];
    State_act(state, &actions[childi]);

    float score = -1 * iterate(child, state);

    root->visits++;
    root->value += score;
    return score;
}

void mcts(const struct State* state,
    struct MCTSResults* r,
    const struct MCTSOptions* o)
{
    results = r;
    memset(results, 0, sizeof(struct MCTSResults));

    if (o == NULL) {
        MCTSOptions_default(&options);
    } else {
        options = *o;
    }

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(state, actions);

    // TODO
    if (actionc == 0) {
        fprintf(stderr, "Can't run MCTS on state with no actions\n");
        return;
    }

    struct Node* root = mctsmalloc(sizeof(struct Node));
    Node_init(root, 0);
    Node_expand(root, state, actionc);

    struct timeval start;
    gettimeofday(&start, NULL);

    int last_actioni = -1;
    while (1) {
        struct State s;
        struct TileState ts;
        State_copy(state, &s, &ts);
        iterate(root, &s);
        results->stats.iterations++;

        results->score = -INFINITY;
        for (int a = 0; a < actionc; a++) {
            float score = -1 * root->children[a]->value / root->children[a]->visits;

            if (score >= results->score) {
                results->score = score;
                results->actioni = a;
            }
        }

        if (last_actioni != results->actioni) {
            results->stats.change_iterations = results->stats.iterations;
        }
        last_actioni = results->actioni;

        if (options.seconds) {
            struct timeval now;
            gettimeofday(&now, NULL);
            uint64_t elapsed = now.tv_sec - start.tv_sec;
            if (elapsed > options.seconds) {
                break;
            }
        }
        if (options.iterations) {
            if (results->stats.iterations == options.iterations) {
                break;
            }
        }
    }

    struct timeval end;
    gettimeofday(&end, NULL);
    results->stats.duration = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000;

    for (int i = 0; i < actionc; i++) {
        results->nodes[i] = *root->children[i];
    }

    if (options.save_tree) {
        results->tree = root;
    } else {
        Node_free(root);
        results->tree = NULL;
    }
}
