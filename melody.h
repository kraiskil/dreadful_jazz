#pragma once
#include <stdint.h>
#include "midi.h"
#define SEED_LEN 16
extern uint8_t seed[SEED_LEN];
uint8_t melody_next_sym(uint8_t seed[SEED_LEN], float temperature);


