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
// These are needed only because onnx2c had a bug where the internal status
// was not reset between sequences
extern float tensor_lstm_Y_h[1][1][HIDDEN_SIZE];
extern float tensor_lstm_Y_c[1][1][HIDDEN_SIZE];

extern uint16_t random_number();

void melody_next_sym(uint8_t seed[SEED_LEN], float temp, uint8_t generated[BATCH_SIZE])
{
	float tensor_input[1][SEED_LEN][VOCAB_SIZE];
	float tensor_output[1][VOCAB_SIZE];


	// One end sequence -> generate only end sequences
	bool is_end=false;
	for( int i=SEED_LEN-BATCH_SIZE; i<SEED_LEN; i++)
		if(seed[i] == MIDI_END )
			is_end=true;
	if( is_end ) {
		for(int i=0; i<BATCH_SIZE; i++)
			generated[i] = MIDI_END;
		return;
	}

	// Reset the network, and re-initialize with the seed, discarding the output:
	// Remove these two lines: see comment on line 36 above
	//memset(tensor_lstm_Y_h, 0, sizeof(tensor_lstm_Y_h));
	//memset(tensor_lstm_Y_c, 0, sizeof(tensor_lstm_Y_c));
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

	for(int b=0; b<BATCH_SIZE; b++) {
		led_on(LED_GREEN);
		entry(tensor_input, tensor_output);
		led_off(LED_GREEN);
		generated[b] = onehot_to_midi(tensor_output, temp);

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
