#include <stdint.h>
#include "vocab.h"

#define MIDI_REST 0
#define MIDI_END 1
#define MIDI_CONT 2

float midinote_to_freq(uint8_t x);
uint8_t onehot_to_midi(float rv[1][VOCAB_SIZE], float temperature);
void midi_to_onehot(uint8_t midi, float rv[1][VOCAB_SIZE]);

