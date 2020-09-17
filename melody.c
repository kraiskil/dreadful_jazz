#include "melody.h"
#include "midi.h"

#if SEED_LEN == 8
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, 69, MIDI_CONT };
#elif SEED_LEN == 4
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT };
#elif SEED_LEN == 16
uint8_t seed[SEED_LEN] = { 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 69, MIDI_CONT, MIDI_CONT, MIDI_CONT, 66,66,66,66,60, MIDI_CONT, 60, MIDI_CONT};
#else
#error
#endif
void entry(float tensor_input_1[1][1][47], float tensor_dense[1][47]);


uint8_t melody_next_sym(uint8_t seed[SEED_LEN], float temp)
{
	float tensor_input[1][1][47];
	float tensor_output[1][47];

	if( seed[SEED_LEN-1] == MIDI_END )
		return MIDI_END;

	for( int i=0; i<SEED_LEN; i++ )
	{
		midi_to_onehot(seed[i], tensor_input[0]);
		entry(tensor_input, tensor_output);
	}
	uint8_t new_midi_note = onehot_to_midi(tensor_output, temp);

	return new_midi_note;
}

