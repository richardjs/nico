#ifndef STATEUTIL_H
#define STATEUTIL_H

#include "state.h"

void State_derive(struct State* state);

// Returns >0 if states are not the same
unsigned int State_compare(const struct State* s1, const struct State* s2);
bool Action_compare(const struct Action* a1, const struct Action* a2);

bool State_valid_action(const struct State* state, const struct Action* action);

enum Player State_stack_player(const struct State* state, const struct Coords* coord);

bool State_terminal(const struct State* state);

#endif
