#include "leds.h"
#include "melody.h"
#include "midi.h"
#include <string.h>
#include <stdbool.h>
#include <libopencm3/stm32/gpio.h>

#if SEED_LEN == 8
uint8_t seed[SEED_LEN] = {60, 61, 62, 63, 64, 65, 66, MIDI_CONT };
#elif SEED_LEN == 4
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT };
#elif SEED_LEN == 16
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 66,66,66,66,60, MIDI_CONT, 60, MIDI_CONT};
#elif SEED_LEN == 24
uint8_t seed[SEED_LEN] = { 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
60, MIDI_CONT, 62, MIDI_CONT, 60, MIDI_CONT, 62, MIDI_CONT,
};
#elif SEED_LEN == 32
uint8_t seed[SEED_LEN] = { 
MIDI_END, MIDI_END,MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 66,66,66,66,60, MIDI_CONT, 60, MIDI_CONT};
#elif SEED_LEN == 40
uint8_t seed[SEED_LEN] = { 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
60, MIDI_CONT, MIDI_CONT, MIDI_CONT, 60, MIDI_CONT, MIDI_CONT, MIDI_CONT 
};
#else
#error
#endif

void entry(float tensor_input_1[1][SEED_LEN][VOCAB_SIZE], float tensor_dense[1][VOCAB_SIZE]);

extern uint16_t random_number();

static float tensor_input[1][SEED_LEN][VOCAB_SIZE];
static float tensor_output[1][VOCAB_SIZE];

void melody_next_sym(uint8_t seed[SEED_LEN], float temp, uint8_t generated[BATCH_SIZE])
{

#if 0
	for( int i=0; i<SEED_LEN; i++ )
	{
		midi_to_onehot(seed[i], tensor_input[0][i]);
		gpio_set(GPIOD, GPIO12);
		entry(tensor_input, tensor_output);
		gpio_clear(GPIOD, GPIO12);
	}

	// Generate BATCH_SIZE of new notes
	for(int i=0; i<BATCH_SIZE-1; i++) {
		generated[i] = onehot_to_midi(tensor_output, temp);
		memcpy(tensor_input, tensor_output, sizeof(tensor_input));
		entry(tensor_input, tensor_output);
	}
	generated[BATCH_SIZE-1] = onehot_to_midi(tensor_output, temp);
#endif
	for( int i=0; i<SEED_LEN; i++ )
		midi_to_onehot(seed[i], tensor_input[0][i]);

	bool at_end = seed[SEED_LEN-1] == MIDI_END;
	for(int b=0; b<BATCH_SIZE; b++) {
		if( at_end ) {
			generated[b] = MIDI_END;
			led_on(LED_RED);
		}
		else {
			led_on(LED_GREEN);
			entry(tensor_input, tensor_output);
			led_off(LED_GREEN);
			generated[b] = onehot_to_midi(tensor_output, temp);
		}

		// push new note to seeds, popping the oldest away
		for( int i=0; i<SEED_LEN-1; i++ ) {
			seed[i] = seed[i+1];
			midi_to_onehot(seed[i], tensor_input[0][i]);
		}
		seed[SEED_LEN-1] = generated[b];
	}
}

void init_seed(void)
{
#warning this is highly fragile and breaks when vocabulary or seed lenght changes
#if SEED_LEN == 40
	// current vocabulary 4 first entries are nice notes
	int16_t noteidx = random_number() & 0x3;
	seed[32] = vocab[noteidx];
	noteidx = random_number() & 0x3;
	seed[36] = vocab[noteidx];

#else

#endif
}
