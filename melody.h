#pragma once
#include <stdint.h>
#include "midi.h"
#define SEED_LEN 40
#define BATCH_SIZE 8
extern uint8_t seed[SEED_LEN];
void melody_next_sym(uint8_t seed[SEED_LEN], float temperature, uint8_t generated[BATCH_SIZE]);


