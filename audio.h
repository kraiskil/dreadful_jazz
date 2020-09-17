
#define Fs 8000
// size in single channel samples. i.e. 2*frames
// Size is 1/16 note, i.e. Fs/4
// (since bpm is fixed at 60, so one 1/4 note duration
// is 1s)
// TODO: assert Fs/4
#define AUDIO_DMA_SIZE (2*(Fs/4))


/* Margin to add to the DMA buffer so that
 * for any generated frequency, all possible
 * phase values are available and selectable
 * with an offset from the audio buffer start,
 * so that there always is AUDIO_DMA_SIZE samples
 * available after the offset.
 * AUDIO_BUFFER_MARGIN is calculated with
 * floor( Fs/min(f) )
 * Fs=8k, lowest audio frequency: 100Hz -> 80
 */
#define AUDIO_BUFFER_MARGIN (2*80)

#define AUDIO_BUFFER_SIZE (AUDIO_BUFFER_MARGIN + AUDIO_DMA_SIZE)

/* Attack, decay, sustain and release - profiles for the audio
 * generation */
enum adsr {
	ADSR_BEGIN_AND_END, // Single 1/16th note
	ADSR_CONTINUE,      // middle of a 3/16'th or longer note
	ADSR_BEGIN,         // start of a note
	ADSR_END,           // end of a note
};

void audio_fill_buffer(int16_t *buffer, float freq, enum adsr);

