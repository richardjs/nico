#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errorcodes.h"
#include "state.h"
#include "stateio.h"

#define VERSION "v.1a"

enum Command {
    NONE,
    RANDOM,
    NORMALIZE,
    LIST_ACTIONS,
    ACT
};

int main(int argc, char* argv[])
{
    fprintf(stderr, "Nico %s (built %s %s)\n", VERSION, __DATE__, __TIME__);

    time_t seed = time(NULL);
    srand(seed);

    init_coords();

    enum Command command = NONE;

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "vnltsrxa:i:c:w:j:k:z:b:d:p:u:o:e:")) != -1) {
        switch (opt) {
        case 'v':
            return 0;
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

        struct State state;
        State_from_string(&state, argv[optind]);

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

        case NORMALIZE:
            State_normalize(&state);
            State_to_string(&state, state_string);
            printf("%s\n", state_string);
            return 0;

        case LIST_ACTIONS:
            for (int i = 0; i < actionc; i++) {
                Action_print(&actions[i], stdout);
            }
            return 0;

        case ACT:
            State_act(&state, &action);
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
