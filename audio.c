#include <assert.h>
#include <math.h>
#include <stdint.h>

#include "audio.h"


#define VOL 0x0100
#define D16(x) ((int16_t)(x*VOL) )

// save phase across calls so the audio signal 
// is smooth between DMA buffer swaps
static float carry_over_phase=0;
static float phase_step;
static float cur_freq;

static inline float adsr_modifier( int i, enum adsr adsr)
{
	// This is BEGIN+END profile
	int attack_len = 128;
	int decay_len = 128;
	int sustain_len = 1408;
	int release_len = 384;
	float attack_level = 2.0f;
	float sustain_level = 1.0f;
	float amp;

	if (i < attack_len) {
		amp = attack_level * i / attack_len;
	}
	else if (i < (attack_len + decay_len) ) {
		int dec_i_left = decay_len - (i - attack_len);
		amp = sustain_level + (attack_level - sustain_level) * (dec_i_left) / decay_len;
	}
	else if (i < (attack_len + decay_len + sustain_len) ) {
		amp = sustain_level;
	}
	else {
		int rel_i_left = release_len - (i - (attack_len + decay_len + sustain_len));
		amp = sustain_level * rel_i_left / release_len;
	}
	return amp;
}

int audio_fill_buffer(int16_t *audio, float freq, enum adsr adsr)
{
	cur_freq = freq;
	phase_step = freq/Fs;
	carry_over_phase=0;

	float phase=0;
	int i;
	for( i=0; i<AUDIO_BUFFER_SIZE/2; i++)
	{
		if( phase >= 1 )
			phase=0;
		float amp = sinf(phase*2*M_PI);
		amp *= adsr_modifier(i, adsr);
		audio[2*i+0] = D16(amp);
		audio[2*i+1] = D16(amp);
		phase+=phase_step;

	}

	return audio_next_phase();
}

int audio_next_phase(void)
{

	float sampl_per_cycle = Fs/cur_freq;
	int full_cycles_per_dma = floor( (AUDIO_DMA_SIZE/2) / sampl_per_cycle); 
	float residual_phase = carry_over_phase + ((AUDIO_DMA_SIZE/2)*phase_step) - full_cycles_per_dma;
	if( residual_phase > 1 )
		residual_phase -= 1;

	float num_steps = residual_phase / phase_step;
	carry_over_phase = residual_phase;
	return (int)num_steps;

#if 0
	float sampl_per_cycle = Fs/cur_freq;
	int full_cycles_per_dma = floor( (AUDIO_DMA_SIZE/2) / sampl_per_cycle); 
	float residual_phase = carry_over_phase + ((AUDIO_DMA_SIZE/2)*phase_step) - full_cycles_per_dma;
	if( residual_phase > 1 )
		residual_phase -= 1;

	int i=0;
	while( (i-1)*phase_step < residual_phase )
		i++;
	carry_over_phase = residual_phase;
	return i;
#endif

#if 0
	int resultant = AUDIO_DMA_SIZE/2 - sampl_per_cycle * full_cycles_per_dma + carry_over_phase;
	if( resultant >= sampl_per_cycle )
		resultant -= sampl_per_cycle;
	carry_over_phase = resultant;
	return carry_over_phase;
#endif
}

