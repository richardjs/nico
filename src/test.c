#include "state.h"
#include "stateio.h"
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

    struct State state;
    struct Action actions[MAX_ACTIONS];

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

        if (memcmp(&state, &translated, sizeof(struct State)) != 0) {
            puts("Something went wrong in state translation");
        }
    }

    // Test normalization
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

    // Test normalization when wrapping
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

    puts("Done!");

    return 0;
}
