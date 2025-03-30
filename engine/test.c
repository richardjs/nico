#include "mcts.h"
#include "state.h"
#include "stateio.h"
#include "stateutil.h"
#include "tile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void State_translate(struct State* state, enum Direction direction);
void tile_coords_to_string(const struct Coords coords[], char string[]);
int flood_fill_empty_hexes(const bool tiles[][GRID_SIZE], const struct Coords* start, bool flood[][GRID_SIZE]);
enum Player State_early_winner(const struct State* state);

void State_print_raw_tile_grid(struct State* state)
{
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int q = 0; q < GRID_SIZE; q++) {
            printf("%d", state->tile_state->tiles[q][r]);
        }
        printf("\n");
    }
}

void State_print_raw_stack_grid(struct State* state)
{
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int q = 0; q < GRID_SIZE; q++) {
            printf("%d", state->stacks[q][r]);
        }
        printf("\n");
    }
}

void State_print_regions(struct State* state)
{
    for (int i = 0; i < state->regionc; i++) {
        printf("region %d s=%d a=%d(%d)/%d(%d)\n",
            i,
            state->region_size[i],
            state->region_available[i][P1],
            state->region_single_available[i][P1],
            state->region_available[i][P2],
            state->region_single_available[i][P2]);
    }
}

int main()
{
    puts("Nico tests...");

    time_t seed = time(NULL);
    srand(seed);

    init_coords();

    struct State state;
    struct TileState tile_state;
    struct Action action;
    struct Action actions[MAX_ACTIONS];
    int actionc;

    char state_string[STATE_STRING_SIZE];

    struct MCTSResults results;

    // Translate around and back to the same place
    {
        // Place first tile
        State_new(&state, &tile_state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        struct State translated = state;

        State_translate(&translated, NORTHWEST);
        State_translate(&translated, SOUTHWEST);
        State_translate(&translated, SOUTH);
        State_translate(&translated, SOUTHEAST);
        State_translate(&translated, NORTHEAST);
        State_translate(&translated, NORTH);

        int c = State_compare(&state, &translated);
        if (c) {
            printf("Something went wrong in state translation (%d)\n", c);
        }
    }

    // Normalization
    {
        State_new(&state, &tile_state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        State_normalize(&state);

        struct State translated = state;

        State_translate(&translated, SOUTH);
        State_translate(&translated, SOUTHEAST);
        State_translate(&translated, SOUTH);
        State_normalize(&translated);

        if (State_compare(&state, &translated) != 0) {
            puts("State different after normalization and translation");
        }
    }

    // Normalization when wrapping
    {
        State_new(&state, &tile_state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        State_normalize(&state);

        struct State translated = state;

        State_translate(&translated, NORTHWEST);
        State_normalize(&translated);

        if (State_compare(&state, &translated)) {
            puts("State different after normalization and translation when wrapping");

            if (memcmp(state.tile_state->tiles, translated.tile_state->tiles, sizeof(bool) * GRID_SIZE * GRID_SIZE) != 0) {
                puts("...difference in tiles");
                State_print_raw_tile_grid(&state);
                printf("\n");
                State_print_raw_tile_grid(&translated);
            }
        }
    }

    // Normalize a new board without crashing or getting stuck
    {
        State_new(&state, &tile_state);
        State_normalize(&state);
    }

    // Derive and compare
    {
        State_new(&state, &tile_state);

        struct State derived = state;
        State_derive(&derived);

        int c = State_compare(&state, &derived);
        if (c) {
            printf("Discrepency deriving and comparaing (compare %d)\n", c);
            State_print(&state, stdout);

            State_to_string(&state, state_string);
            printf("State string %s\n", state_string);
        }

        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        derived = state;
        State_derive(&derived);

        c = State_compare(&state, &derived);
        if (c) {
            printf("Discrepency deriving and comparaing (compare %d)\n", c);
            State_print(&state, stdout);

            State_to_string(&state, state_string);
            printf("State string %s\n", state_string);
        }
    }

    // Check some basic branching factors
    {
        State_new(&state, &tile_state);
        for (int i = 0; i < 8; i++) {
            State_actions(&state, actions);
            State_act(&state, &actions[0]);
        }

        State_normalize(&state);

        actionc = State_actions(&state, actions);
        if (actionc != 22) {
            printf("Initial place action count %d != 21\n", actionc);
            State_print(&state, stdout);
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
        }

        State_act(&state, &actions[0]);
        actionc = State_actions(&state, actions);

        if (actionc != 21) {
            printf("Second initial place action count %d != 20\n", actionc);
            State_print(&state, stdout);
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
        }

        State_act(&state, &actions[0]);

        actionc = State_actions(&state, actions);
        if (actionc != 15) {
            printf("Branching factor here %d != 30\n", actionc);
            State_print(&state, stdout);
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
        }
    }

    // Stack move bug
    {
        char test_state_string[] = "0,3|0,4|0,5|1,2|1,3|1,4|1,5|2,2|2,3|2,4|3,0|3,1|3,2|3,3|4,0|4,1|4,2|4,3|5,2|5,3|5,5|5,6|6,2|6,3|6,4|6,5|7,1|7,2|7,4|7,5|8,3|8,4|1,5h16|1,2t16|h";
        char test_action_string[] = "1,5|3|0,5";

        State_from_string(&state, &tile_state, test_state_string);
        Action_from_string(&action, test_action_string);

        State_act(&state, &action);

        if (state.stacks[0][5] != 3 || state.stacks[1][5] != INITIAL_STACK - 3) {
            puts("Stacks isn't what it should be");
        }
    }

    // Terminal state
    {
        char test_state_string[] = "0,11|1,10|1,11|2,10|3,9|3,10|4,8|4,9|5,1|5,6|5,7|6,0|6,1|6,2|6,6|6,7|6,8|6,9|7,0|7,1|7,2|7,4|7,5|7,8|7,9|8,1|8,2|8,3|8,4|9,1|9,2|10,1|10,1h2|5,1h6|9,1h1|8,2h1|7,4h1|9,2h1|8,4h1|8,3h1|7,0t2|6,0t5|6,1t1|7,2t2|8,1t3|7,1t1|6,2t2|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (!State_terminal(&state)) {
            puts("Didn't detect terminal state");
        }
    }
    {
        // This is a different state from above, and it should *not* be terminal
        char test_state_string[] = "0,11|1,10|1,11|2,10|3,9|3,10|4,8|4,9|5,1|5,6|5,7|6,0|6,1|6,2|6,6|6,7|6,8|6,9|7,0|7,1|7,2|7,4|7,5|7,8|7,9|8,1|8,2|8,3|8,4|9,1|9,2|10,1|10,1h2|5,1h6|9,1h1|8,2h1|7,4h1|9,2h1|8,4h3|8,3h1|7,0t2|6,0t5|6,1t1|7,2t2|8,1t3|7,1t1|6,2t2|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (State_terminal(&state)) {
            puts("Wrongly detected terminal state");
        }
    }

    // Normalize and calulate actions for long board ("edge" case, ha) without hanging
    {
        char test_state_string[] = "0,23|0,22|1,22|1,21|2,20|2,19|3,19|3,18|4,17|4,16|5,16|5,15|6,14|6,13|7,13|7,12|8,11|8,10|9,10|9,9|10,8|10,7|11,7|11,6|12,5|12,4|13,4|13,3|14,2|14,1|15,1|15,0|15,0h3|h";
        State_from_string(&state, &tile_state, test_state_string);
        State_normalize(&state);
        int actionc = State_actions(&state, actions);
        if (actionc != 4) {
            puts("Incorrect number of actions for long state");
        }
    }

    // Softserve issue #9
    {
        char test_state_string[] = "0,0|0,1|1,0|1,1|t";
        State_from_string(&state, &tile_state, test_state_string);

        int actionc = State_actions(&state, actions);
        bool success = false;
        for (int i = 0; i < actionc && !success; i++) {
            struct State after;
            struct TileState after_tiles;
            State_copy(&state, &after, &after_tiles);
            State_act(&after, &actions[i]);

            if (after.tile_state->tiles[2][1]
                && after.tile_state->tiles[2][2]
                && after.tile_state->tiles[3][0]
                && after.tile_state->tiles[3][1]) {
                success = true;
            }
        }

        if (!success) {
            puts("Not able to place tiles connected by middle hexes");
        }
    }

    // Softserve issue #9, part 2
    {
        char test_state_string[] = "1,2|1,3|2,2|2,3|t";
        State_from_string(&state, &tile_state, test_state_string);

        int actionc = State_actions(&state, actions);
        bool success = false;
        for (int i = 0; i < actionc && !success; i++) {
            struct State after;
            struct TileState after_tiles;
            State_copy(&state, &after, &after_tiles);
            State_act(&after, &actions[i]);

            if (after.tile_state->tiles[0][1]
                && after.tile_state->tiles[1][0]
                && after.tile_state->tiles[1][1]
                && after.tile_state->tiles[2][0]) {
                success = true;
            }
        }

        if (!success) {
            puts("Not able to place tiles connected by middle hexes, part 2");
        }
    }

    // flood fill
    {
        State_new(&state, &tile_state);
        struct Coords start = { .q = 0, .r = 0 };
        bool flood[GRID_SIZE][GRID_SIZE];
        int count = flood_fill_empty_hexes(state.tile_state->tiles, &start, flood);

        if (count != GRID_SIZE * GRID_SIZE) {
            puts("empty flood fill did not get expected count");
        }

        char test_state_string[] = "1,0|1,1|0,1|0,0|1,2|1,3|0,3|0,2|3,3|3,4|2,4|2,3|5,2|5,3|4,3|4,2|3,-1|3,0|2,0|2,-1|5,-1|5,0|4,0|4,-1|7,0|7,1|6,1|6,0|7,2|7,3|6,3|6,2|h";
        State_from_string(&state, &tile_state, test_state_string);

        start.q = 2;
        start.r = 1;

        count = flood_fill_empty_hexes(state.tile_state->tiles, &start, flood);
        if (count != 6) {
            printf("hole flood fill incorrect: %d != 6\n", count);
        }

        start.q = 0;
        start.r = 4;

        count = flood_fill_empty_hexes(state.tile_state->tiles, &start, flood);
        if (count != 638) {
            printf("open flood fill incorrect: %d != 638\n", count);
        }
    }

    // Restrict start place actions to edges
    {
        char test_state_string[] = "1,0|1,1|0,1|0,0|1,2|1,3|0,3|0,2|3,3|3,4|2,4|2,3|5,2|5,3|4,3|4,2|3,-1|3,0|2,0|2,-1|5,-1|5,0|4,0|4,-1|7,0|7,1|6,1|6,0|7,2|7,3|6,3|6,2|h";
        State_from_string(&state, &tile_state, test_state_string);

        int actionc = State_actions(&state, actions);
        if (actionc != 22) {
            printf("Incorrect number of actions for stack place: %d\n", actionc);
        }
    }

    // Winner detection
    {
        char test_state_string[] = "0,7|1,4|1,5|1,6|1,7|1,8|2,4|2,5|2,6|2,7|2,8|3,1|3,3|3,4|3,5|3,6|3,7|4,0|4,1|4,2|4,3|4,4|4,5|4,6|5,0|5,1|5,5|5,6|6,0|6,1|6,5|7,0|4,1h1|3,1h12|5,0h1|5,1h1|6,0t9|4,0t2|7,0t4|6,1t1|4,2h1|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (State_winner(&state) != P1) {
            puts("incorrect winner for state");
            State_print(&state, stdout);
        }
    }
    {
        char test_state_string[] = "0,7|1,4|1,5|1,6|1,7|1,8|2,4|2,5|2,6|2,7|2,8|3,1|3,3|3,4|3,5|3,6|3,7|4,0|4,1|4,2|4,3|4,4|4,5|4,6|5,0|5,1|5,5|5,6|6,0|6,1|6,5|7,0|4,1h1|3,1h13|5,0t1|5,1h1|6,0t9|4,0t2|7,0t4|6,1h1|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (State_winner(&state) != DRAW) {
            puts("incorrect winner for state");
            State_print(&state, stdout);
        }
    }
    {
        char test_state_string[] = "0,7|1,4|1,5|1,6|1,7|1,8|2,4|2,5|2,6|2,7|2,8|3,1|3,3|3,4|3,5|3,6|3,7|4,0|4,1|4,2|4,3|4,4|4,5|4,6|5,0|5,1|5,5|5,6|6,0|6,1|6,5|7,0|4,1h1|3,1h13|5,0h1|5,1h1|6,0t9|4,0t2|4,2h1|4,3t1|7,0t4|6,1t1|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (State_winner(&state) != P1) {
            puts("incorrect winner for state");
            State_print(&state, stdout);
        }
    }

    // MCTS bug
    {
        char test_state_string[] = "2,7|3,6|3,7|4,6|4,5|3,5|4,4|5,4|6,3|6,2|7,2|7,1|5,5|6,4|6,5|5,6|6,1|6,0|5,1|5,2|4,3|3,4|3,3|4,2|1,8|1,7|0,8|0,9|2,5|1,6|1,5|2,4|4,6h1|1,5t1|3,7h13|6,5t14|3,6h2|1,8t1|h";
        State_from_string(&state, &tile_state, test_state_string);

        State_actions(&state, actions);

        mcts(&state, &results, NULL);
        action = actions[results.actioni];

        if (action.start.q == 3 && action.start.r == 6 && action.end.q == 2 && action.end.r == 7) {
            puts("This is definitely not the move to do here!");
            State_print(&state, stdout);
            Action_print(&action, stdout);
            State_act(&state, &action);
            State_print(&state, stdout);
        }
    }

    // Regions
    {
        char test_state_string[] = "3,2|4,2|3,3|2,3|4,0|5,0|4,1|3,1|2,5|3,4|3,5|2,6|6,1|7,1|6,2|5,2|7,-1|8,-1|7,0|6,0|6,3|7,2|7,3|6,4|8,4|8,3|9,3|9,4|11,3|12,3|11,4|10,4|7,3h16|3,3t16|t";
        State_from_string(&state, &tile_state, test_state_string);

        if (state.regionc != 3) {
            printf("incorrect number of regions: %d\n", state.regionc);
            State_print(&state, stdout);
        }
    }
    {
        char test_state_string[] = "1,0|2,0|1,1|0,1|2,1|3,0|3,1|2,2|0,3|1,2|1,3|0,4|4,2|4,1|5,1|5,2|4,4|4,3|5,3|5,4|7,3|6,3|7,2|8,2|2,5|1,5|2,4|3,4|7,1|6,1|7,0|8,0|h";
        State_from_string(&state, &tile_state, test_state_string);

        if (state.regionc != 1) {
            printf("incorrect number of regions: %d\n", state.regionc);
            State_print(&state, stdout);
        }

        action.start.q = 4;
        action.start.r = 1;
        action.count = INITIAL_STACK;
        State_act(&state, &action);

        if (state.regionc != 2) {
            printf("incorrect number of regions: %d\n", state.regionc);
            State_print(&state, stdout);
        }

        action.start.q = 4;
        action.start.r = 4;
        action.count = INITIAL_STACK;
        State_act(&state, &action);

        if (state.regionc != 2) {
            printf("incorrect number of regions: %d\n", state.regionc);
            State_print(&state, stdout);
        }

        action.start.q = 4;
        action.start.r = 1;
        action.end.q = 4;
        action.end.r = 3;
        action.count = 1;
        State_act(&state, &action);

        if (state.regionc != 3) {
            printf("incorrect number of regions: %d\n", state.regionc);
            State_print(&state, stdout);
        }
    }

    // Early termination
    {
        char test_state_string[] = "6,1|7,1|6,2|5,2|5,1|4,1|5,0|6,0|3,4|4,3|4,4|3,5|3,6|4,5|4,6|5,5|1,8|0,8|1,7|2,7|3,7|2,8|3,8|4,7|2,3|2,4|1,4|1,3|2,2|2,1|3,1|3,2|3,4h16|3,5t16|h";
        State_from_string(&state, &tile_state, test_state_string);

        if (State_early_winner(&state) != NO_WINNER) {
            puts("Detected this as an early win:");
            State_print(&state, stdout);
        }
    }
    {
        char test_state_string[] = "6,1|7,1|6,2|5,2|5,1|4,1|5,0|6,0|3,4|4,3|4,4|3,5|3,6|4,5|4,6|5,5|1,8|0,8|1,7|2,7|3,7|2,8|3,8|4,7|2,3|2,4|1,4|1,3|2,2|2,1|3,1|3,2|3,4h8|3,5t16|4,4h8|h";
        State_from_string(&state, &tile_state, test_state_string);

        State_early_winner(&state);
        if (State_early_winner(&state) != NO_WINNER) {
            puts("Detected this as an early win:");
            State_print(&state, stdout);
        }
    }
    {
        char test_state_string[] = "6,1|7,1|6,2|5,2|5,1|4,1|5,0|6,0|3,4|4,3|4,4|3,5|3,6|4,5|4,6|5,5|1,8|0,8|1,7|2,7|3,7|2,8|3,8|4,7|2,3|2,4|1,4|1,3|2,2|2,1|3,1|3,2|3,4h15|3,5t16|4,4h1|h";
        State_from_string(&state, &tile_state, test_state_string);

        State_early_winner(&state);
        if (State_early_winner(&state) != P1) {
            puts("Should have detected this as P1 win:");
            State_print(&state, stdout);
        }
    }

    // This shouldn't evaluate as -inf
    //{
    //    char test_state_string[] = "1,4|2,4|1,5|0,5|3,3|3,2|4,2|4,3|3,4|4,4|3,5|2,5|5,3|6,2|6,3|7,2|1,3|1,2|2,2|2,3|5,1|5,0|6,0|6,1|3,1|2,1|3,0|4,0|5,4|5,5|6,4|6,5|6,0h1|4,4t4|1,5h2|1,4t1|3,5h3|6,2t1|3,0h1|2,4t1|3,3h1|3,4t1|3,2h1|2,5t1|0,5h2|2,3t1|1,2h1|1,3t2|2,2h1|5,3t2|4,2h1|4,3t1|3,1h1|6,4t1|6,1h1|t";
    //    State_from_string(&state, &tile_state, test_state_string);
    //    State_print(&state, stdout);
    //    State_print_regions(&state);
    //    printf("%d\n", State_early_winner(&state));
    //}

    puts("Done!");

    return 0;
}
