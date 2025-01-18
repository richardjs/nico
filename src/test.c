#include <stdio.h>
#include "state.h"
#include "stateio.h"

int main()
{
    puts("Nico tests");

    struct State state;
    State_new(&state);

    struct Action actions[MAX_ACTIONS];
    int actionc = State_actions(&state, actions);

    State_act(&state, &actions[0]);
    State_print(&state, stdout);

    State_new(&state);
    State_act(&state, &actions[1]);
    State_print(&state, stdout);

    // TODO this is broken because half the tile wraps around and the output isn't normalized
    State_new(&state);
    State_act(&state, &actions[2]);
    State_print(&state, stdout);

    return 0;
}
