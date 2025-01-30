#ifndef STATEUTIL_H
#define STATEUTIL_H

#include "state.h"

void State_derive(struct State* state);

// Returns >0 if states are not the same
unsigned int State_compare(const struct State* s1, const struct State* s2);

#endif
