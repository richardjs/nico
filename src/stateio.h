#ifndef STATEIO_H
#define STATEIO_H

#include "state.h"
#include <stdio.h>

void State_normalize(struct State* state);
void State_print(const struct State* state, FILE* stream);

#endif
