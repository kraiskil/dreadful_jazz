#include <math.h>
#include <stdint.h>
#include <libopencm3/stm32/adc.h>
#include "midi.h"

/* verbatim from libopencm3-examples. GPL */
static uint16_t read_adc_naiive(uint8_t channel)
{
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}

// Output of NN -> MIDI note.
// 0 : rest
// 1 : end
// 2 : continuation (i.e. _)
#include "vocab.h"

float midinote_to_freq(uint8_t x)
{
	int a=440;
	return a * powf(2, (float)(x-69)/12);
}

uint8_t onehot_to_midi(float rv[1][VOCAB_SIZE], float temperature)
{
	float max=rv[0][0];
	uint8_t maxi=0;
	uint8_t maxi2=0;
	// TODO: proper temperature sampling

	for(int i=1; i<VOCAB_SIZE; i++) {
		if( rv[0][i] > max ) {
			max = rv[0][i];
			maxi2=maxi;
			maxi = i;
		}
	}

	if( (read_adc_naiive(0) & 0x7)  < 3 ){
		// its a bit eager in selecting END
		//if( vocab[maxi2] == MIDI_END )
		//	return vocab[maxi];
		//else
			return vocab[maxi2];
	}
	else
		return vocab[maxi];
}

void midi_to_onehot(uint8_t midi, float rv[1][VOCAB_SIZE])
{
	for( int i=0; i<VOCAB_SIZE; i++ )
		if( vocab[i] == midi )
			rv[0][i]=1;
		else
			rv[0][i]=0;
}

