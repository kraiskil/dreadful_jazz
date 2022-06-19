#pragma once
#include <libopencm3/stm32/gpio.h>
enum board_leds
{
	LED_GREEN = GPIO12,  // NN inference time
	LED_ORANGE = GPIO13, // "idle time" (waiting for audio buffers to play before filling them again)
	LED_RED = GPIO14,    // end of melody/error/...
	LED_BLUE = GPIO15    // IS2 DMA ISR, this is toggled on each ISR
};

void led_on( enum board_leds );
void led_off( enum board_leds );
void led_toggle( enum board_leds );

