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
    // int actionc;

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
        }

        State_actions(&state, actions);
        State_act(&state, &actions[0]);

        derived = state;
        State_derive(&derived);

        c = State_compare(&state, &derived);
        if (c) {
            printf("Discrepency deriving and comparaing (compare %d)\n", c);
            State_print(&state, stdout);
        }
    }

    // Basic serialization->deserialization cases
    {
        State_new(&state);
        // TODO increase i once we have more types of actions
        for (int i = 0; i < 5; i++) {
            State_actions(&state, actions);
            State_act(&state, &actions[0]);

            // State_to_string normalizes the state, so normalize it here so we can compare it later
            State_normalize(&state);

            State_to_string(&state, state_string);
            struct State from_string_state;
            State_from_string(&from_string_state, state_string);

            int c = State_compare(&state, &from_string_state);
            if (c) {
                printf("Discrepency serializing and deserializing state (compare %d)\n", c);
                State_print(&state, stdout);
                break;
            }
        }
    }

    puts("Done!");

    return 0;
}
