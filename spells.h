#pragma once
#include "state.h"

typedef state(*transistion)(state s);

const transistion* get_transistions();
const char** get_trans_names();
void damage_init();
double damage_get();
void damage_reset();
void disabled(int* d, state s);

#define num_spells 7
