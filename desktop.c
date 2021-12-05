#include <stdio.h>
#include "midi.h"

void entry(float tensor_input_1[1][1][VOCAB_SIZE], float tensor_dense[1][VOCAB_SIZE]);

float tensor_input[1][1][VOCAB_SIZE];
float tensor_output[1][VOCAB_SIZE];

int main(void)
{
	midi_to_onehot(55, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);

	midi_to_onehot(69, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(69, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);

	midi_to_onehot(67, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);
	entry(tensor_input, tensor_output);
	midi_to_onehot(2, tensor_input[0]);

	for(int i=0; i<10; i++ ) {
		entry(tensor_input, tensor_output);
		uint8_t curr_midi_note = onehot_to_midi(tensor_output);
		midi_to_onehot(curr_midi_note, tensor_input[0]);

		printf("%d\n", (int) curr_midi_note);
	}

	
}
