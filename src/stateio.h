#ifndef STATEIO_H
#define STATEIO_H

#include "state.h"
#include <stdio.h>

#define P1_CHAR 'h'
#define P2_CHAR 't'

#define STATE_STRING_SIZE 435

void State_normalize(struct State* state);

// Console rendering
void State_print(const struct State* state, FILE* stream);
// void Action_print(const struct Action* action, FILE* stream);

// Serialized representations
bool State_from_string(struct State* state, const char string[]);
void State_to_string(const struct State* state, char string[]);
void Action_from_string(struct Action* action, const char string[]);
void Action_to_string(const struct Action* action, char string[]);

#endif
