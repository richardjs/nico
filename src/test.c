#include "state.h"
#include "stateio.h"
#include "stateutil.h"
#include "tile.h"
#include <stdio.h>
#include <string.h>

void State_translate(struct State* state, enum Direction direction);
void tile_coords_to_string(const struct Coords coords[], char string[]);

void State_print_raw_tile_grid(struct State* state)
{
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int q = 0; q < GRID_SIZE; q++) {
            printf("%d", state->tile_state->tiles[q][r]);
        }
        printf("\n");
    }
}

int main()
{
    puts("Nico tests...");
    init_coords();

    struct State state;
    struct TileState tile_state;
    struct Action action;
    struct Action actions[MAX_ACTIONS];
    int actionc;

    char state_string[STATE_STRING_SIZE];

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

        if (State_compare(&state, &translated)) {
            puts("Something went wrong in state translation");
        }
    }

    // Normalization
    {
        State_new(&state, &tile_state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        struct State translated = state;

        State_translate(&translated, SOUTH);
        State_translate(&translated, SOUTHEAST);
        State_translate(&translated, SOUTH);
        State_normalize(&translated);

        if (memcmp(&state, &translated, sizeof(struct State)) != 0) {
            puts("State different after normalization and translation");

            if (memcmp(state.tile_state->tiles, translated.tile_state->tiles, sizeof(bool) * GRID_SIZE * GRID_SIZE) != 0) {
                puts("...difference in tiles");
                State_print_raw_tile_grid(&state);
                printf("\n");
                State_print_raw_tile_grid(&translated);
            }
        }
    }

    // Normalization when wrapping
    {
        State_new(&state, &tile_state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        struct State translated = state;

        State_translate(&translated, NORTHWEST);
        State_normalize(&translated);

        if (memcmp(&state, &translated, sizeof(struct State)) != 0) {
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
        if (actionc != 32) {
            State_print(&state, stdout);
            printf("Initial place action count %d != 32\n", actionc);
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
        }

        State_act(&state, &actions[0]);
        actionc = State_actions(&state, actions);

        if (actionc != 31) {
            State_print(&state, stdout);
            printf("Second initial place action count %d != 31\n", actionc);
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
        }

        State_act(&state, &actions[0]);

        actionc = State_actions(&state, actions);
        if (actionc != 30) {
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

    // Tile permutations
    {
        struct Tile tile;
        tile.origin.q = 0;
        tile.origin.r = 0;
        tile.direction = TILE_SOUTHEAST;

        char coords_string[ACTION_STRING_SIZE];

        puts("original:");
        struct Coords coords[TILE_SIZE];
        Tile_coords(&tile, coords);
        tile_coords_to_string(coords, coords_string);
        printf("%s\n", coords_string);

        puts("---");

        struct Coords permutations[TILE_PERMUTATIONS][TILE_SIZE];
        Tile_permutations(&tile, permutations);

        for (int i = 0; i < TILE_PERMUTATIONS; i++) {
            tile_coords_to_string(&permutations[i][0], coords_string);
            printf("%s\n", coords_string);
        }
    }

    puts("Done!");

    return 0;
}
