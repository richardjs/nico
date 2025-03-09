#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errorcodes.h"
#include "mcts.h"
#include "state.h"
#include "stateio.h"
#include "stateutil.h"
#include "think.h"
#include "tile.h"

#define VERSION "v.1a"

enum Command {
    NONE,
    PRINT,
    NORMALIZE,
    LIST_ACTIONS,
    WINNER,
    THINK,
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

    char* action_arg;

    enum Command command = NONE;

    struct MCTSOptions options;
    MCTSOptions_default(&options);

    int workers = 1;

    int opt;
    struct Action action;
    while ((opt = getopt(argc, argv, "viwIPnlWtra:")) != -1) {
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

        case 't':
            command = THINK;
            break;

        case 'i':
            options.iterations = atoi(optarg);
            break;

        case 'w':
            workers = atoi(optarg);
            break;

        case 'W':
            command = WINNER;
            break;

        case 'r':
            command = RANDOM;
            break;

        case 'a':
            command = ACT;
            action_arg = optarg;
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
                if (actions[i].count != 0) {
                    Action_print(&actions[i], stdout);
                    continue;
                }

                // TODO clean this up
                struct Tile tile = { .origin = actions[i].start, .direction = actions[i].end.q };
                struct Coords permutations[TILE_PERMUTATIONS][TILE_SIZE];
                Tile_permutations(&tile, permutations);
                for (int j = 0; j < TILE_PERMUTATIONS; j++) {
                    char tile_string[ACTION_STRING_SIZE];
                    tile_coords_to_string(&permutations[j][0], tile_string);
                    printf("%s\n", tile_string);
                }
            }

            return 0;

        case WINNER:
            if (actionc != 0) {
                puts("none");
                return 0;
            }
            switch (State_winner(&state)) {
            case P1:
                puts("h");
                break;
            case P2:
                puts("t");
                break;
            case DRAW:
                puts("draw");
                break;
            }
            return 0;

        case ACT:
            // Parse potential tile action
            // TODO This is an unabashed quick fix
            for (int i = 0; i < actionc; i++) {
                // If the action isn't a tile place, print it normally
                if (actions[i].count != 0) {
                    continue;
                }

                struct Tile tile = { .origin = actions[i].start, .direction = actions[i].end.q };
                struct Coords permutations[TILE_PERMUTATIONS][TILE_SIZE];
                Tile_permutations(&tile, permutations);
                for (int j = 0; j < TILE_PERMUTATIONS; j++) {
                    char tile_string[ACTION_STRING_SIZE];
                    tile_coords_to_string(&permutations[j][0], tile_string);

                    if (strcmp(action_arg, tile_string) == 0) {
                        action = actions[i];
                        goto action_parsed;
                    }
                }
            }

            Action_from_string(&action, action_arg);
        action_parsed:

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

        case THINK:
            break;
        }

        struct MCTSResults results;
        think(&state, &results, &options, workers);

        const struct Action* selected_action;
        if (results.presearch_action) {
            selected_action = results.presearch_action;
        } else {
            selected_action = &actions[results.actioni];
        }

        // TODO clean this up (as well as above)
        if (selected_action->count != 0) {
            Action_print(selected_action, stdout);
        } else { 
            struct Tile tile = { .origin = selected_action->start, .direction = selected_action->end.q };
            struct Coords permutations[TILE_PERMUTATIONS][TILE_SIZE];
            Tile_permutations(&tile, permutations);
            char tile_string[ACTION_STRING_SIZE];
            tile_coords_to_string(&permutations[0][0], tile_string);
            printf("%s\n", tile_string);
        }

        State_act(&state, selected_action);
        State_print(&state, stderr);

        return 0;
    }
}
