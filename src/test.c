#include "state.h"
#include "stateio.h"
#include "stateutil.h"
#include <stdio.h>
#include <string.h>

void State_translate(struct State* state, enum Direction direction);

void State_print_raw_tile_grid(struct State* state)
{
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int q = 0; q < GRID_SIZE; q++) {
            printf("%d", state->tiles[q][r]);
        }
        printf("\n");
    }
}

int main()
{
    puts("Nico tests...");
    init_coords();

    struct State state;
    struct Action actions[MAX_ACTIONS];
    int actionc;

    char state_string[STATE_STRING_SIZE];

    // Translate around and back to the same place
    {
        // Place first tile
        State_new(&state);
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
        State_new(&state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        struct State translated = state;

        State_translate(&translated, SOUTH);
        State_translate(&translated, SOUTHEAST);
        State_translate(&translated, SOUTH);
        State_normalize(&translated);

        if (memcmp(&state, &translated, sizeof(struct State)) != 0) {
            puts("State different after normalization and translation");

            if (memcmp(state.tiles, translated.tiles, sizeof(bool) * GRID_SIZE * GRID_SIZE) != 0) {
                puts("...difference in tiles");
                State_print_raw_tile_grid(&state);
                printf("\n");
                State_print_raw_tile_grid(&translated);
            }
        }
    }

    // Normalization when wrapping
    {
        State_new(&state);
        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        struct State translated = state;

        State_translate(&translated, NORTHWEST);
        State_normalize(&translated);

        if (memcmp(&state, &translated, sizeof(struct State)) != 0) {
            puts("State different after normalization and translation when wrapping");

            if (memcmp(state.tiles, translated.tiles, sizeof(bool) * GRID_SIZE * GRID_SIZE) != 0) {
                puts("...difference in tiles");
                State_print_raw_tile_grid(&state);
                printf("\n");
                State_print_raw_tile_grid(&translated);
            }
        }
    }

    // Normalize a new board without crashing or getting stuck
    {
        State_new(&state);
        State_normalize(&state);
    }

    // Derive and compare
    {
        State_new(&state);

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
        State_new(&state);
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

        State_act(&state, &actions[3]);
        State_print(&state, stdout);
    }

    puts("Done!");

    return 0;
}
