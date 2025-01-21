#include "state.h"
#include "stateio.h"
#include <stdio.h>
#include <string.h>

void State_translate(struct State* state, enum Direction direction);

int main()
{
    puts("Nico tests...");

    struct State state;
    struct Action actions[MAX_ACTIONS];

    // Translate around and back to the same place
    {
        // Place first tile
        State_new(&state);
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

    puts("Done!");

    return 0;
}
