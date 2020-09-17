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
	int decay_len = 512 - attack_len;
	int sustain_len = 512;
	int release_len = 1024;
	float attack_level = 2.0f;
	float sustain_level = 0.6f;
	float amp;

	int samples = AUDIO_DMA_SIZE/2;
	if( adsr == ADSR_CONTINUE ) {
		attack_len=0;
		decay_len=0;
		sustain_len = samples;
		release_len=0;
	}
	else if( adsr == ADSR_BEGIN ) {
		sustain_len = samples - (attack_len+decay_len);
		release_len = 0;
	}
	else if( adsr == ADSR_END ) {
		attack_len=0;
		decay_len=0;
		sustain_len=samples - release_len;
	}

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

void audio_fill_buffer(int16_t *audio, float freq, enum adsr adsr)
{
	cur_freq = freq;
	phase_step = freq/Fs;
	float phase;
	if( adsr == ADSR_BEGIN || adsr == ADSR_BEGIN_AND_END )
		phase=0;
	else
		phase = carry_over_phase;

	int i;
	for( i=0; i<AUDIO_DMA_SIZE/2; i++)
	{
		if( phase >= 1.0 )
			phase-=1.0;
		float amp = sinf(phase*2*M_PI);
		amp *= adsr_modifier(i, adsr);
		audio[2*i+0] = D16(amp);
		audio[2*i+1] = D16(amp);
		phase+=phase_step;
	}
	carry_over_phase = phase;
}
