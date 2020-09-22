#include "melody.h"
#include "midi.h"
#include <string.h>

#if SEED_LEN == 8
uint8_t seed[SEED_LEN] = {60, 61, 62, 63, 64, 65, 66, MIDI_CONT };
#elif SEED_LEN == 4
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT };
#elif SEED_LEN == 16
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 66,66,66,66,60, MIDI_CONT, 60, MIDI_CONT};
#elif SEED_LEN == 24
uint8_t seed[SEED_LEN] = { 
MIDI_END, MIDI_END, MIDI_END, MIDI_END,  MIDI_END, MIDI_END, MIDI_END, MIDI_END,
MIDI_END, MIDI_END, MIDI_END, MIDI_END,  MIDI_END, MIDI_END, MIDI_END, MIDI_END,
60, MIDI_CONT, MIDI_CONT, MIDI_CONT, 60, MIDI_CONT, MIDI_CONT, MIDI_CONT };
#elif SEED_LEN == 32
uint8_t seed[SEED_LEN] = { 
MIDI_END, MIDI_END,MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, MIDI_END, 
69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 66,66,66,66,60, MIDI_CONT, 60, MIDI_CONT};
#else
#error
#endif
void entry(float tensor_input_1[1][1][VOCAB_SIZE], float tensor_dense[1][VOCAB_SIZE]);
extern float tensor_lstm_Y_h[1][1][HIDDEN_SIZE];
extern float tensor_lstm_Y_c[1][1][HIDDEN_SIZE];


void melody_next_sym(uint8_t seed[SEED_LEN], float temp, uint8_t generated[BATCH_SIZE])
{
	float tensor_input[1][1][VOCAB_SIZE];
	float tensor_output[1][VOCAB_SIZE];

	memset(tensor_lstm_Y_h, 0, sizeof(tensor_lstm_Y_h));
	memset(tensor_lstm_Y_c, 0, sizeof(tensor_lstm_Y_c));

	// One end sequence -> generate only end sequences
	if( seed[SEED_LEN-1] == MIDI_END ) {
		for(int i=0; i<BATCH_SIZE; i++)
			generated[i] = MIDI_END;
		return;
	}

	for( int i=0; i<SEED_LEN; i++ )
	{
		midi_to_onehot(seed[i], tensor_input[0]);
		entry(tensor_input, tensor_output);
	}
	for(int i=0; i<BATCH_SIZE-1; i++) {
		generated[i] = onehot_to_midi(tensor_output, temp);
		memcpy(tensor_input, tensor_output, sizeof(tensor_input));
		entry(tensor_input, tensor_output);
	}
	generated[BATCH_SIZE-1] = onehot_to_midi(tensor_output, temp);
}

