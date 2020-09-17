#include <math.h>
#include <stdint.h>
#include <libopencm3/stm32/adc.h>

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
uint8_t vocab[47] = {
64, 49, 81, 78, 45, 66, 70, 43, 88, 47, 65, 63, 61,
82, 72, 51, 84, 85, 60, 76, 71, 58, 79, 74, 57, 54,
75, 59,  0, 86, 73, 93, 52,  1, 56, 55, 77,  2, 48,
68, 80, 62, 50, 83, 53, 67, 69};

float midinote_to_freq(uint8_t x)
{
	int a=440;
	return a * powf(2, (float)(x-69)/12);
}

uint8_t onehot_to_midi(float rv[1][47], float temperature)
{
	float max=rv[0][0];
	uint8_t maxi=0;
	uint8_t maxi2=0;
	// TODO: proper temperature sampling

	for(int i=1; i<47; i++) {
		if( rv[0][i] > max ) {
			max = rv[0][i];
			maxi2=maxi;
			maxi = i;
		}
	}
	if( (read_adc_naiive(0) & 0x7)  == 0 )
		return vocab[maxi2];
	else
		return vocab[maxi];
}

void midi_to_onehot(uint8_t midi, float rv[1][47])
{
	for( int i=0; i<47; i++ )
		if( vocab[i] == midi )
			rv[0][i]=1;
		else
			rv[0][i]=0;
}

