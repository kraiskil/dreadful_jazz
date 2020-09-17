/*
 * Example for how to use I2C and I2S with libopencm3.
 * This is intended for the STM32F411-Discovery demoboard.
 *
 * The device plays a 1kHz sine through the audio jack.
 */

/* Compile time option to use DMA to feed audio to the I2S.
 * Without this, the audio is sent by polling the I2S peripheral's
 * TXE bit */
#define USE_DMA


#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/dma.h>

#include <math.h>

#include "audio.h"
#include "midi.h"
#include "melody.h"




int16_t audio_0[AUDIO_BUFFER_SIZE];
int16_t audio_1[AUDIO_BUFFER_SIZE];
bool audio_playing_0 = true;

float curr_note_freq=1000;
uint8_t curr_midi_note=50;

/* Timer to count played note parts: 1/16th notes being the resolution.
 * This is incremented in sync with the DMA sending the buffer to I2S.
 * Spin-wait for this to increment to find the start end/start of a DMA frame */
volatile uint32_t note_increments=0;

static void write_i2c_to_audiochip( uint8_t reg, uint8_t contents)
{
	uint8_t packet[2];
	packet[0] = reg;
	packet[1] = contents;
	/* STM32F411 discovery user's manyal gives device address with R/W bit,
	 * libopencm wants it without it */
	uint8_t address = (0x94)>>1;

	i2c_transfer7(I2C1, address, packet, 2, NULL, 0);
}

/* Interrupt service routine for the DMA.
 * The name is "magic" and suffices as installation hook into libopencm3.
 * NB: this WILL block sys_tick_handler() ISR, even if the systick has higher
 * priority. Seems OpenCM3 ISRs have attribute "blocking_handler" set,
 * which sounds like the ISRs are run with interrupts disabled. So be quick here!
 */
void dma1_stream5_isr(void)
{

	/* Clear the 'transfer complete' interrupt, or execution would jump right back to this ISR. */
	if (dma_get_interrupt_flag(DMA1, DMA_STREAM5, DMA_TCIF)) {
		dma_clear_interrupt_flags(DMA1, DMA_STREAM5, DMA_TCIF);
	}


	//int offs = 2*audio_next_phase();
	if( dma_get_target(DMA1, DMA_STREAM5) == 0 )
	{
		audio_playing_0=true;
	//	dma_set_memory_address_1(DMA1, DMA_STREAM5, (uint32_t) (audio_0 + offs)); //2* because its stereo
	}
	else
	{
		audio_playing_0=false;
	//	dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t) (audio_0 + offs));
	}
	note_increments++;
}


/* monotonically increasing number of milliseconds from reset
 * overflows every 49 days if you're wondering
 */
volatile uint32_t system_millis;

/* Called when systick fires */
void sys_tick_handler(void)
{
	system_millis++;
}

/* sleep for delay milliseconds */
#if 0
static void msleep(uint32_t delay)
{
	uint32_t wake = system_millis + delay;
	while (wake > system_millis);
}
static void sleep_to(uint32_t time)
{
	while (time > system_millis);
}
#endif

/* Set up a timer to create 1mS ticks. */
static void systick_setup(void)
{
	/* clock rate / 1000 to get 1mS interrupt rate */
	systick_set_reload(84000);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
	systick_counter_enable();
	/* this done last */
	systick_interrupt_enable();
}




int main(void)
{
	/* Set device clocks from opencm3 provided preset.*/
	const struct rcc_clock_scale *clocks = &rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ];
	rcc_clock_setup_pll( clocks );

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOD);

	/* Initialize "heartbeat" LED GPIOs. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12); /* green led */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13); /* orange led */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14); /* red led */


	/* I2C GPIO pins
	 * PB6 - SCL (I2C clock)
	 * PB9 - SDA (I2C data)
	 * The board does not have pullups on the I2C lines, so
	 * we use the chip internal pullups.
	 * Also the pins must be open drain, as per I2C specification.
	 * STM32F411 datasheet "Alternate Functions table" tells that
	 * I2C is AlternateFucntion 4 for both pins.
	 */
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO6);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO9);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO6);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO9);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO6);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO9);


	/* Initialize the I2C itself.
	 * Since we are master, we would not need to initialize slave
	 * address, but this is the only libopencm3 API call that sets
	 * the 'bit14 of CCR' - a bit in the I2C that is HW reset to 0,
	 * but manual says 'must be 1' */
	rcc_periph_clock_enable(RCC_I2C1);
	i2c_peripheral_disable(I2C1);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, clocks->apb1_frequency/1000000);
	i2c_set_own_7bit_slave_address(I2C1, 0);
	i2c_peripheral_enable(I2C1);


	/* Initialize I2S.
	 * I2S is implemented as a HW mode of the SPI peripheral.
	 * Since this is a STM32F411, there is a separate I2S PLL
	 * that needs to be enabled.
	 */
	rcc_osc_on(RCC_PLLI2S);
	rcc_periph_clock_enable(RCC_SPI3);
	i2s_disable(SPI3);
	i2s_set_standard(SPI3, i2s_standard_philips);
	i2s_set_dataformat(SPI3, i2s_dataframe_ch16_data16);
	i2s_set_mode(SPI3, i2s_mode_master_transmit);
	i2s_masterclock_enable(SPI3);
	/* RCC_PLLI2SCFGR is left at reset value: 0x24003010 i.e.
	 * PLLR = 2
	 * PLLI2SN = 192
	 * PLLI2SM = 16
	 * And since the input is PLL source (i.e. HSI = 16MHz)
	 * The I2S clock = 16 / 16 * 192 / 2 = 96MHz
	 * Calculate sampling frequency from equation given in
	 * STM32F411 reference manual:
	 * Fs = I2Sclk/ (32*2 * ((2*I2SDIV)+ODD)*4)
	 * I2SDIV = I2Sclk/(512*Fs)
	 * Fs=8kHz => I2SDIV=23,4 so 23 + ODD bit set
	 */
	i2s_set_clockdiv(SPI3, 23, 1);
#ifdef USE_DMA
	/* Have the SPI/I2S peripheral ping the DMA each time data is sent.
	 * The DMA peripheral is configured later. */
	spi_enable_tx_dma(SPI3);
#endif
	i2s_enable(SPI3);


	/* I2S pins:
	 * Master clock: PC7
	 * Bit clock: PC10
	 * Data: PC12
	 * L/R clock: PA4
	 */
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
	gpio_set_af(GPIOC, GPIO_AF6, GPIO7);
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO10);
	gpio_set_af(GPIOC, GPIO_AF6, GPIO10);
	gpio_mode_setup(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO12);
	gpio_set_af(GPIOC, GPIO_AF6, GPIO12);
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO4);
	gpio_set_af(GPIOA, GPIO_AF6, GPIO4);


	/* Initialize the Audio DAC, as per its datasheet.
	 * CS43L22 /RESET is connected to PD4, first release it. Then write
	 * minimum set of needed settings. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
	gpio_set(GPIOD, GPIO4);
	write_i2c_to_audiochip(0x06, 0x04); // interface control 1: set I2S dataformat
	write_i2c_to_audiochip(0x02, 0x9e); // power control 1: Magic value to power up the chip


	/* Enable DMA from memory to I2S peripheral.
	 * The DMA is configured as circular, i.e. it restarts automatically when
	 * the requested amount of datasamples are set.
	 * SPI3/I2S3 is available on DMA1 stream 5, channel 0 (see RM 0383, table 27) */
	rcc_periph_clock_enable(RCC_DMA1);
	nvic_enable_irq(NVIC_DMA1_STREAM5_IRQ);
	dma_disable_stream(DMA1, DMA_STREAM5);
	dma_set_priority(DMA1, DMA_STREAM5, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA1, DMA_STREAM5, DMA_SxCR_MSIZE_16BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM5, DMA_SxCR_PSIZE_16BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM5);
	dma_enable_circular_mode(DMA1, DMA_STREAM5);
	dma_enable_double_buffer_mode(DMA1, DMA_STREAM5);
	dma_set_transfer_mode(DMA1, DMA_STREAM5, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA1, DMA_STREAM5, (uint32_t) &SPI_DR(SPI3));
	dma_set_number_of_data(DMA1, DMA_STREAM5, AUDIO_DMA_SIZE);
	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM5);
	dma_channel_select(DMA1, DMA_STREAM5, DMA_SxCR_CHSEL_0);

	systick_setup();

	audio_fill_buffer(audio_0, 0, ADSR_CONTINUE);
	dma_set_memory_address(DMA1, DMA_STREAM5, (uint32_t) audio_0);
	dma_set_memory_address_1(DMA1, DMA_STREAM5, (uint32_t) audio_1);

	dma_enable_stream(DMA1, DMA_STREAM5);


	/* ADC setup as in the libopencm3-examples */
	rcc_periph_clock_enable(RCC_ADC1);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO0);
	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO1);
	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);
	adc_power_on(ADC1);

	

	float nfreq=0;
	uint32_t now = note_increments;
	while(1) {
		// wait for DMA transfer to I2S to finish
		while( note_increments <= now );
		/* Blink the heartbeat LED at at 2xbpm*/
		gpio_toggle(GPIOD, GPIO13);
		now = note_increments;

		enum adsr adsr;

		// play first seed
		uint8_t nnote = seed[0];
		if( nnote == MIDI_REST) {
			nfreq = 0;
			adsr = ADSR_BEGIN_AND_END;
		}
		else if( nnote == MIDI_END ) {
			gpio_set(GPIOD, GPIO14); // red led
			adsr = ADSR_BEGIN_AND_END;
			nfreq = 0;
		}
		else if ( nnote != MIDI_CONT ) {
			nfreq = midinote_to_freq(nnote);
			adsr = ADSR_BEGIN;
		}
		else { // This is a midi continue note
			if( seed[1] == MIDI_CONT )
				adsr = ADSR_CONTINUE;
			else
				adsr = ADSR_END;
		}

		int16_t *fillbuff = audio_playing_0 ? audio_1 : audio_0;
		audio_fill_buffer(fillbuff, nfreq, adsr);
		
		// add new note to end of seed
		gpio_set(GPIOD, GPIO12);
		//nnote = melody_next_sym(seed, 0.8);
		nnote = MIDI_END;
		gpio_clear(GPIOD, GPIO12);
		for( int i=1; i<SEED_LEN; i++)
			seed[i-1] = seed[i];
		
		seed[SEED_LEN-1]=nnote;


	}
	return 0;
}

