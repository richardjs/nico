#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errorcodes.h"
#include "state.h"
#include "stateio.h"
#include "stateutil.h"

#define VERSION "v.1a"

enum Command {
    NONE,
    PRINT,
    NORMALIZE,
    LIST_ACTIONS,
    RANDOM,
    ACT
};

int main(int argc, char* argv[])
{
    fprintf(stderr, "Nico %s (built %s %s)\n", VERSION, __DATE__, __TIME__);

    time_t seed = time(NULL);
    srand(seed);

    init_coords();

    struct State state;
    struct TileState tile_state;

    enum Command command = NONE;

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "vIPnltsrxa:i:c:w:j:k:z:b:d:p:u:o:e:")) != -1) {
        switch (opt) {
        case 'v':
            return 0;

        case 'I':
            State_new(&state, &tile_state);
            char state_string[STATE_STRING_SIZE];
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            return 0;

        case 'P':
            command = PRINT;
            break;

        case 'n':
            command = NORMALIZE;
            break;

        case 'l':
            command = LIST_ACTIONS;
            break;

        case 'r':
            command = RANDOM;
            break;

        case 'a':
            command = ACT;
            Action_from_string(&action, optarg);
            break;
        }

        if (argc == optind) {
            fprintf(stderr, "No state provided\n");
            return ERROR_NO_STATE_GIVEN;
        }

        State_from_string(&state, &tile_state, argv[optind]);

        fprintf(stderr, "input: %s\n", argv[optind]);
        State_print(&state, stderr);

        struct Action actions[MAX_ACTIONS];
        int actionc = State_actions(&state, actions);

        char state_string[STATE_STRING_SIZE];
        char action_string[ACTION_STRING_SIZE];

        switch (command) {
        case NONE:
            fprintf(stderr, "No command given\n");
            return ERROR_NO_COMMAND_GIVEN;

        case PRINT:
            State_print(&state, stdout);
            return 0;

        case NORMALIZE:
            State_normalize(&state);
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            return 0;

        case LIST_ACTIONS:
            if (actionc == 0) {
                if (State_terminal(&state)) {
                    puts("terminal state");
                } else {
                    puts("no actions");
                }
                return 0;
            }

            for (int i = 0; i < actionc; i++) {
                // If the action isn't a tile place, print it normally
                if (actions[i].count == 0) {
                    Action_print(&actions[i], stdout);
                }

                // TODO here
            }

            return 0;

        case ACT:
            State_act(&state, &action);

            // Check if we need to skip turns
            if (State_actions(&state, actions) == 0) {
                fprintf(stderr, "Skipping turn for %c\n", state.turn == P1 ? P1_CHAR : P2_CHAR);
                State_act(&state, NULL);
            }

            State_normalize(&state);
            State_to_string(&state, state_string);
            State_print(&state, stderr);
            printf("%s\n", state_string);
            return 0;

        case RANDOM:
            struct Action* action = &actions[rand() % actionc];

            Action_to_string(action, action_string);
            printf("%s\n", action_string);

            State_act(&state, action);
            State_normalize(&state);

            State_print(&state, stderr);
            State_to_string(&state, state_string);
            fprintf(stderr, "next:\t%s\n", state_string);
            return 0;
        }

        return 0;
    }
}
