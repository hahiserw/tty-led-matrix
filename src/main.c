#include <stdio.h> // file
#include <stdlib.h> // printf

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>

#include "usb_serial.h"

// xxx remove some
#include "main.h"
#include "window.h"
#include "console.h"
#include "screen.h"


uint8_t annoying_dog[16 * 4] = {
	5,232,0,0,10,20,0,0,8,4,0,0,8,2,0,0,18,67,4,0,16,0,202,0,17,128,58,0,21,32,2,0,19,192,2,0,16,0,2,0,48,0,3,0,64,0,0,128,96,0,1,0,128,0,0,128,64,0,0,64,63,255,255,128
};

// 2 out of 2.5KB on atmega32u4
uint8_t main_buffer[2048] = {0};
uint8_t main_window = 0;

#if 0
void scroll(void) {
// #define columns DISPLAY_WIDTH
// #define size DISPLAY_WIDTH * DISPLAY_HEIGHT
	// uint8_t c, d;
	// for (uint16_t l = 0; l < size; l+= columns) {
	// 	c = 0;
	// 	for (uint8_t p = columns; p;)
	// 	{
	// 		d = display[--p + l] >> (8-n);
	// 		display[p + l] = c | (display[p+l] << n);
	// 		c = d;
	// 	}
	// }

}
#endif

// doesn't look good
// ISR(TIMER0_OVF_vect) {
// 	sw_scroll_tick();
// }

// ISR(TIMER1_COMPA_vect) {
// 	// sw_scroll_tick();
// 	scan();
// 	// 0..10 for 16MHz, otherwise flashing is noticable
// 	for (uint8_t i = 0; i < 5; i++)
// 		empty_scan();
// }


// I like printf
int ld_putchar(char c, FILE *stream) { return usb_serial_putchar(c); }
int ld_getchar(FILE *stream) { return usb_serial_getchar(); }
FILE uart = FDEV_SETUP_STREAM(ld_putchar, ld_getchar, _FDEV_SETUP_RW);

void hardware_init(void) {
	CPU_PRESCALE(0);

	// doesn't work ;_;
	stdout = &uart;
	stderr = &uart;

	// scroll timer

	// TCCR1A = (1 << WGM01); // compare
	// // TCCR0B = (1 << CS02) | (0 << CS01) | (1 << CS00); // clk / 0, 1, 8, 64, 256, 1024
	// TCCR1B = 5;
	// TIMSK0 = (1 << TOIE0); // overflow
	// // TIMSK1 = (1 << OCIE0A); // compare
	// // OCR1A = 65535;

	// brightness counter
	//

	DDRB = 0x0f | (1 << 5); // brightness
	// DDRD = 0xff;
	DDRD = 0x0f;
	DDRF = 0x03;

	PIN_OE(1);
	PIN_LAT(0);

#ifdef SPI_IFACE
	// disable SPI power saving?
	PRR0 &= ~(1 << PRSPI);
	// master, enable, fosc/4
	SPCR = (1 << MSTR) | (1 << SPE);
#endif

	// wdt_reset();
	// WDTCSR = (1 << WDE) | (1 << WDP2);
}


int main(void)
{
	hardware_init();

#define LAYOUT_FULL 0
#define LAYOUT_SPLIT_VERTICALY 1

// #define LAYOUT LAYOUT_FULL
#define LAYOUT LAYOUT_SPLIT_VERTICALY

#if LAYOUT == LAYOUT_FULL
	sw *csw = sw_new(0, 0,
					 DISPLAY_WIDTH, DISPLAY_HEIGHT,
					 DISPLAY_WIDTH * 4, DISPLAY_HEIGHT,
					 NO_SCROLL);

	sw_sorted[0] = csw;
#elif LAYOUT == LAYOUT_SPLIT_VERTICALY
	sw_sorted[0] = sw_new(0, 0,
					 DISPLAY_WIDTH, DISPLAY_HEIGHT / 2,
					 DISPLAY_WIDTH * 2, DISPLAY_HEIGHT / 2,
					 NO_SCROLL);

	sw_sorted[1] = sw_new(0, DISPLAY_HEIGHT / 2,
					 DISPLAY_WIDTH, DISPLAY_HEIGHT / 2,
					 DISPLAY_WIDTH * 2, DISPLAY_HEIGHT / 2,
					 NO_SCROLL);

	sw *csw = sw_sorted[0];
#endif

	if (csw == NULL)
		die();

	// sw_scroll(csw, SCROLL_BUFFER_START); // show data immediately

// #define FONT_TEST
// #define WELCOME_TEXT
// #define BARK
#define VERT_TEST

#ifdef WELCOME_TEXT
	uint8_t *m = (uint8_t *)
#  if FONT_HEIGHT / 16 >= 2
		"„Zakaz wędzonej kiełbasy mnie rozjusza”\n"
		"                              -- Wojciech Cejrowski"
#    if FONT_HEIGHT / 16 >= 3
		"\n"
#    endif
#  else
		// "„Zakaz wędzonej kiełbasy mnie rozjusza”   -- Wojciech Cejrowski"
		"    "
#  endif
		;
	draw_text(csw, m);
#endif

#ifdef FONT_TEST
	for (uint8_t i = 32; i < 128; i++) {
#  if FONT_HEIGHT / 16 >= 3
		if (i % 32 == 0)
			parse_letter(csw, '\n');
#  endif
		parse_letter(csw, i);
	}
#endif

#ifdef BARK
	for (uint8_t i = 0; i < 16; i++)
		memcpy(&buffer2d(csw, i, 0), &annoying_dog[4 * i], 4);
#endif

#ifdef VERT_TEST
	draw_text(sw_sorted[0], "„Zakaz wędzonej kiełbasy mnie rozjusza”");
	sw_sorted[0]->scroll_mode = SCROLL_LEFT;
#if LAYOUT == LAYOUT_SPLIT_VERTICALY
	// draw_text(sw_sorted[1], "                              -- Wojciech Cejrowski");
	draw_text(sw_sorted[1], "-- Wojciech Cejrowski");
	// sw_sorted[1]->scroll_mode = SCROLL_LEFT;
	sw_sorted[1]->scroll_mode = SCROLL_RIGHT;
#endif
#endif

	usb_init();

	sei();

	int16_t c = -1;

	for (;;) {
		// wdt_reset();

		csw = &sw_set[main_window];

		// XXX shouldn't it be handled by an interrupt?
		// so text can be rendered instead of just shoving zeros down spi
		scan();
		// 0..10 for 16MHz, otherwise flashing is noticable
		for (uint8_t i = 0; i < 8; i++)
			empty_scan();
		// not a good way to do it

		// if (y == 15) {
		c = usb_serial_getchar();

		// write letter into display buffer
		// use special codes to toggle scrolling/changin font sizes and stuff
		if (c >= 0)
			parse_letter(csw, c);
		// }
	}

	return 0;
}


void die() {
	PIN_OE(1);
	for (;;)
		;
}
