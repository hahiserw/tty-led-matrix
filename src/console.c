#include <stdint.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "console.h"
#include "screen.h" // shouldn't be here I presume
#include "window.h" // temp, just for sw_sorted
#include "font.h" // font_map

#define FONT_HEIGHT  csw->font->height
#define FONT_WIDTH   csw->font->width

// #define FONT_SIZE 16
// #define FONT_SIZE 8

// #define CONSOLE_DISCARD_EXCESIVE_CHARACTERS


#if FONT_SIZE == 5
#  include "../../laser/src/font.c"
#elif FONT_SIZE == 6
#  include "font-4x6.c"
#  define FONT_IN_PGM
#  define FONT_HEIGHT FONT_CHAR_HEIGHT
#  define FONT_WIDTH  FONT_CHAR_WIDTH
#  define TINYFONT_SPRITE
#elif FONT_SIZE == 8
// #  include "../fonts/font-5x8.c"
#  include "../fonts/5x8.c"
#elif FONT_SIZE == 13
//#  include "../fonts/font-6x13-all.c"
#  include "../fonts/font-6x13B-all.c"
#elif FONT_SIZE == 14
//#  include "../fonts/font-7x14-all.min.c"
//#  include "../fonts/font-7x14.c"
#  include "../fonts/font-7x14B-all.c"
#elif FONT_SIZE == 15
#  include "../fonts/font-9x15.c"
#elif FONT_SIZE == 16
//#  include "../fonts/ter-u16n.c"
#  include "../fonts/ter-u16b.c"
//#  include "../fonts/ter-u16v.c" // a few characters are wider
//#  define FONT_WIDTH 9 // u16v looks nicer
#endif

// static uint8_t last_c = 0;

void draw_text(sw *csw, char *m)
{
	while (*m)
		// draw_letter(csw, *m++);
		parse_letter(csw, *m++);
}

void parse_new_line(sw *csw) {
	// -1 so it moves the screen just before
	if (csw->cursor_y / FONT_HEIGHT == csw->buffer_height / FONT_HEIGHT - 1) {
		// move the screen up memorywise
		if (csw->height / FONT_HEIGHT >= 2)
			memmove(csw->buffer,
					&buffer2d(csw, FONT_HEIGHT, 0),
					(csw->height - FONT_HEIGHT) * csw->buffer_width / 8);

		// clear the last line
		memset(&buffer2d(csw, (csw->buffer_height - FONT_HEIGHT), 0),
			   0, FONT_HEIGHT * csw->buffer_width / 8);
	} else
	// if (csw->cursor_y + 2 * FONT_HEIGHT < csw->height)
		csw->cursor_y += FONT_HEIGHT;

	csw->cursor_x = 0;
}


// todo?
// typedef enum { PARSE_CHARACTER, PARSE_UNICODE, PARSE_IMAGE } parse;
// parse parse_mode = PARSE_CHARACTER;

#define INFO_LENGTH 100
char info[INFO_LENGTH];

uint8_t wcbytes = 0;
uint32_t wc = 0;

void parse_letter(sw *csw, uint8_t c) {
	// unicode parsing
	if (c >= 0x80) {
		if (c >= 0xf0) {
			wc = c & 0x07;
			wcbytes = 4;
		} else if (c >= 0xe0) {
			wc = c & 0x0f;
			wcbytes = 3;
		} else if (c >= 0xc0) {
			wc = c & 0x1f;
			wcbytes = 2;
		} else {
			wc = (wc << 6) | (c & 0x3f);
			wcbytes--;
		}

		if (wcbytes != 1)
			return;
	}

	// visible characters
	if ((c >= 32 && c < 128) || wcbytes == 1) {
		// scroll left if the text doesn't fit on the screen :D
		// XXX make it configurable
		// if (csw->cursor_x + FONT_WIDTH > csw->width) {
		// 	csw->scroll_mode = SCROLL_LEFT;
		// 	sw_scroll(csw, SCROLL_BUFFER_PRE_START); // won't work at first I think
		// } else {
		// 	csw->scroll_mode = NO_SCROLL;
		// 	sw_scroll(csw, SCROLL_BUFFER_START);
		// }

#ifdef CONSOLE_DISCARD_EXCESIVE_CHARACTERS
		if (csw->cursor_x + FONT_WIDTH > csw->buffer_width)
			// FIXME stops displaying after that
		{
			csw->cursor_x = 0;
			return;
		}
#else
		if (csw->cursor_x + FONT_WIDTH > csw->buffer_width)
			parse_new_line(csw);
#endif

		// or always put data into wc?
		if (wcbytes == 1) {
			draw_letter(csw, wc);
			wcbytes = 0;
		} else
			draw_letter(csw, c);

		csw->cursor_x += FONT_WIDTH;

		// last_c = c;
		return;
	}

	// control characters
	switch (c) {
	case '\r':
		csw->cursor_x = 0;
		break;

	case '\n':
		// FIXME don't go to the next line if cursor is in its last position
		// do so only if there's yet another \n
		// XXX test me
		// static uint8_t another_nl = 0;
		// if (csw->cursor_x == csw->width / FONT_WIDTH && last_c == '\n' && another_nl)
		// 	;
		// else
			parse_new_line(csw);
		break;

	case '\t':
		// put_char(' ') that many times
		// csw->cursor_x +=
		// 	(CONSOLE_TAB_SIZE - ((csw->cursor_x / FONT_WIDTH) % CONSOLE_TAB_SIZE))
		// 	* FONT_WIDTH;
		for (uint8_t i = CONSOLE_TAB_SIZE - ((csw->cursor_x / FONT_WIDTH)
											 % CONSOLE_TAB_SIZE); i; i--)
			parse_letter(csw, ' ');
		break;

	case KEY_CTRL('['): // esc
		// get more chars
		// draw images
		// {
		// upos_t width, height, x, y;
		// char *data; // base64
		// }
		break;

	case '\b':
	case 0x7f:
		// and clear one character
		if (csw->cursor_x >= FONT_WIDTH) {
			csw->cursor_x -= FONT_WIDTH;
			// draw_letter4(csw, ' ');
			for (uint8_t y = 0; y < FONT_HEIGHT; y++)
				for (uint8_t x = 0; x < FONT_WIDTH; x++)
					draw_pixel(csw, csw->cursor_x + x, csw->cursor_y + y, 0);
		}
		break;

	case KEY_CTRL('Q'):
		// fprintf(&uart, "disabling OE, safe to reprogram\r\n");
		die();
		break;
		// // fixme restart µc and go into bootloader
		// *(uint16_t *)0x0800 = 0x7777;
		// cli();
		// UDCON = 1;
		// USBCON = (1<<FRZCLK);
		// UCSR1B = 0;
		// // _delay_ms(5);
		// WDTCSR = (1 << WDE) | (1 << WDP2);
		// for (;;)
		// 	;
		// break;

	case KEY_CTRL('F'):
		for (uint8_t i = 0; i < sw_counter; i++)
			fprintf(&uart, "w: %6p\r\n", &sw_set[i]);

		// for (uint8_t i = 0; i < 4; i++)
		// 	fprintf(&uart, "s: %6p\r\n", sw_sorted[i]);

		for (uint8_t i = 0; i < 10; i++)
			fprintf(&uart, "n: %6p\r\n", sw_next[i]);
		fprintf(&uart, "\r\n");
		break;

	case KEY_CTRL('O'):
		// skip root window
		if (main_window + 1 < sw_counter)
			main_window++;
		else
			main_window = 1;
		break;

		// toggle scrolling (and delay)
	case KEY_CTRL('R'):
		csw->scroll_mode = csw->scroll_mode == NO_SCROLL?
			SCROLL_LEFT: NO_SCROLL;
		if (csw->scroll_mode == NO_SCROLL)
			sw_scroll(csw, SCROLL_BUFFER_START);
		break;

	case KEY_CTRL('W'):
	case KEY_CTRL('E'):
		if (c == KEY_CTRL('W'))
			counter_overflow = (counter_overflow + 1) & 0x1f;
		else
			counter_overflow = (counter_overflow - 1) & 0x1f;

		fprintf(&uart, "speed: %i  \r", counter_overflow);
		counter = 0;
		break;

	case KEY_CTRL('N'):
		fprintf(&uart, "windows: %i\r\n", sw_counter);
		FOREACH_WINDOW(w, i) {
			fprintf(&uart, "%2i: %ix%i+%i+%i\r\n",
					i, w->width, w->height, w->x, w->y);
			// xxx for some reason breaks, even with zeros
			// fprintf(&uart, " %ix%i (%i)\r\n",
			// 		0, 0, 0
			// 		 // w->buffer_width, w->buffer_height,
			// 		 // w->buffer_end - w->buffer
			// 	   );
		}
		break;

	// case KEY_CTRL('K'):
	// 	// todo clear the whole line
	// 	break;

	case KEY_CTRL('S'):
	case KEY_CTRL('T'):
	case KEY_CTRL('U'):
		if (c == KEY_CTRL('U'))
			csw->offset_x = 0;

		fprintf(&uart, "ox: %4i  <: %2i  -: %2i  >: %2i \r",
				csw->offset_x,
				space_pre,
				space_in,
				space_post
				);

		// fprintf(&uart, "ox: %4i  pre: %2i  in: %2i  post: %2i  xm: %2d  d: 0x%02x \r",
		// 		csw->offset_x,
		// 		space_pre,
		// 		space_in,
		// 		space_post,
		// 		xm,
		// 		// ((*csw->buffer << xm) | (*(csw->buffer+1) >> (8 - xm))) & 0xff
		// 		0
		// 		);

		if (c == KEY_CTRL('S'))
			csw->scroll_mode = SCROLL_LEFT;
		else if (c == KEY_CTRL('T'))
			csw->scroll_mode = SCROLL_RIGHT;

		sw_scroll_tick();
		csw->scroll_mode = NO_SCROLL;

		break;

	case KEY_CTRL(']'): // group separator
		break;

	case '\f': // ^L
		// TODO screen_clear(csw);
#define WINDOW_BUFFER_SIZE(csw) \
		(csw->buffer_width * csw->buffer_height / 8)
		memset(csw->buffer, 0, WINDOW_BUFFER_SIZE(csw));
		// TODO console_cursor_move(0, 0);
		csw->cursor_x = 0;
		csw->cursor_y = 0;
		break;
	}

	// last_c = c;
}


// TODO fmt
void console_write(char *text) {
	fprintf(&uart, text);
}


#if FONT_SIZE == 5
// #define FONT_4X4_FOR_REAL
void draw_letter(sw *csw, uint8_t c) {
	if (c < 32 || c > 127)
		return;

	// usb_serial_putchar(':');
	// usb_serial_putchar(c);

	uint8_t i, y, yo, glyph;
	// static uint8_t i, n, yo, glyph;

	// move small letters 1px down
	// should also move ',' and ';', but it doesn't have 5 rows
	if ((c >= 'a' && c <= 'z')
#ifdef FONT_4X4_FOR_REAL
		&& c != 'j' && c != 'g' && c != 'p' && c != 'q' && c != 'y'
#else
		|| (c == ',' || c == ';')
#endif
		)
		yo = 1;
	else
		yo = 0;

	// erase line above the small letter or under the big one
	for (i = 0; i < 4; i++)
		draw_pixel(csw, csw->cursor_x + i, csw->cursor_y + (4 - 4 * yo), 0);

	for (i = 0; i < 4; i++) {
		// TODO just read next bytes here
		// pgm_read_dword?
		glyph = pgm_read_byte(TINYFONT_SPRITE + ((c - 32) / 2) * 4 + i)
			>> (c & 1) * 4;

		for (y = 0; y < 4; y++) {
			draw_pixel(csw, csw->cursor_x + i, csw->cursor_y + y + yo,
					   glyph & (1 << y));
		}
	}

	// for (n = 0; n < 2; n++) {
	// 	glyph = pgm_read_byte(TINYFONT_SPRITE + (c - 32) * 2 + n);
	// 	// glyph = pgm_read_word(TINYFONT_SPRITE + (c - 32) * 2 + n);

	// 	for (i = 0; i < 8; i++) {
	// 		draw_pixel(write_offset + (i & 0x03), i / 4 + n - yo,
	// 				   glyph & (0x80 >> i));
	// 		// usb_serial_putchar(glyph & (0x80 >> i)? '1': '0');
	// 	}
	// }
}
#elif FONT_SIZE == 6
void draw_letter(sw *csw, uint8_t c) {
	uint8_t glyph;

	for (uint8_t y = 0; y < 6; y++) {
		// 2 lines per byte
		glyph = pgm_read_byte(font4x6 + 3 * c + y / 2);

		// fprintf(&uart, "glyph: 0x%02x \r", glyph);

		for (uint8_t x = 0; x < FONT_WIDTH; x++) {
			draw_pixel(csw, csw->cursor_x + x, csw->cursor_y + y,
					   (glyph << 4 * (y & 1)) & (0x80 >> x));
		}
	}
}
#else
#ifdef FONT_UTF8
void draw_letter(sw *csw, uint32_t c) {
	c = font_map(csw->font, c);
#else
void draw_letter(sw *csw, uint8_t c) {
	if (c < 32)
		return;

	c -= 32;
// #ifdef FONT_UTF8
// 	c += 1; // 32 => 1, 33 => 2, 290 => 259
// #endif
#endif

	uint8_t glyph;

	for (uint8_t y = 0; y < FONT_HEIGHT; y++) {
		// 2 lines per byte
		glyph = pgm_read_byte(csw->font->data + FONT_HEIGHT * c + y);

		// fprintf(&uart, "glyph: 0x%02x \r", glyph);

		for (uint8_t x = 0; x < FONT_WIDTH; x++) {
			draw_pixel(csw, csw->cursor_x + x, csw->cursor_y + y,
					   glyph & (0x80 >> x));
		}
	}
}
#endif
