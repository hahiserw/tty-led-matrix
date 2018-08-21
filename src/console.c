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
static uint8_t console_mode = CONSOLE_MODE_DEFAULT;


void draw_text(sw *csw, char *m)
{
	while (*m)
		// draw_letter(csw, *m++);
		parse_letter(csw, *m++);
}

// XXX fix horizontal scrolling
// #define HORIZONTAL_SPLIT_HORZINONTAL_BUFFER

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

#ifdef HORIZONTAL_SPLIT_HORZINONTAL_BUFFER
	if (csw->height / FONT_HEIGHT >= 2) { // && csw->scroll != SCROLL_LEFT || SCROLL_RIGHT
		if (csw->cursor_y / FONT_HEIGHT >= csw->height / FONT_HEIGHT - 1)
			csw->offset_y += FONT_HEIGHT;
	}
#endif
	} else
	// if (csw->cursor_y + 2 * FONT_HEIGHT < csw->height)
		csw->cursor_y += FONT_HEIGHT;

	csw->cursor_x = 0;
}


pos_t console_tailor_arg(pos_t arg, pos_t default_value, pos_t multiplier, pos_t max)
{
	if (!arg)
		return default_value;
	else {
		if (arg * multiplier >= max)
			return max - multiplier;
		else
			return arg * multiplier;
	}
}

// move to font.c?
font_t *font_get_fitting(font_t *f, pos_t height)
{
	// przyjmując that font order is from biggest to smallest
	while (f->next && f->height > height)
		f = f->next;

	return f;
}

#define CONSOLE_BUFFER_SIZE 8
uint8_t console_buffer[CONSOLE_BUFFER_SIZE];
uint8_t *console_buffer_pointer;

void parse_escape_sequence(sw *csw, uint8_t c) {
	if (console_buffer_pointer - console_buffer >= CONSOLE_BUFFER_SIZE - 1) {
		console_mode = CONSOLE_MODE_DEFAULT;
		return;
	}

	*console_buffer_pointer++ = c;

	switch (console_buffer[0]) {
		uint8_t arg0;

	case '[': // OSC
		// switch (console_buffer[1]) {
		// case 'A': break; // move cursor up
		// }
		break;

	// TODO write documentation
	// <Esc> _ <digit> <letter>
	// always needs a decimal arg
	case '_': // APC - application program commands
		// wait for more data
		if (console_buffer_pointer - console_buffer < 3)
			return;

		arg0 = console_buffer[1] - '0';
		if (arg0 < 0 || arg0 > 9)
			break;

		switch (console_buffer[2]) {
			static pos_t split_arg0;
			static pos_t split_arg1;

		case 'g': // font
			if (arg0) {
				while (arg0--) {
					if (!csw->font->next->next)
						break;
					csw->font = csw->font->next;
				}
			} else
				csw->font = font_first;

			sw_reset(csw);
			break;

		case 'r': // scroll mode
			if (arg0 > 4) // SCROLL_MODE_COUNT?
				break;

			csw->scroll_mode = (scroll)arg0;

			// xxx like this? or should user decide. might be configurable
			sw_scroll(csw, SCROLL_BUFFER_START);
			sw_scroll(csw, SCROLL_BUFFER_START_VERT);
			break;

		case 't': // scroll buffer direction
			if (arg0 > 5)
				break;

			sw_scroll(csw, (scroll_buffer)arg0);
			break;

			// TODO repeat showing of buffer when scrolling instead of showing
			// blank
		// case 'u':
		// 	break;

		case 'f': // report configuration
			// 2 ways:
			// - change reporting level for current window
			// - change reporting level to level from split_arg0 on arg0 window

			// XXX that takes a lot of space (0.2%) :o
#if 0
			switch (arg0) {
			case 0: csw->flags &= ~FLAG_REPORT_OVERFLOWS; break;
			case 1: csw->flags |= FLAG_REPORT_OVERFLOWS; break;

			case 2: csw->flags &= ~FLAG_SCROLL_WHEN_OVERFLOW; break;
			case 3: csw->flags |= FLAG_SCROLL_WHEN_OVERFLOW; break;
			}
#endif
			break;

		case 's': // scrolling speed
			if (arg0 < 5)
				counter_overflow = arg0;
			else
				counter_overflow = (arg0 - 4) * 6;
			break;

		// function ideas:
		// - display the message for a few scrolling cycles and clear it?

		// temporary, silly way of passing args
		case 'a': split_arg0 = arg0; break;
		case 'b': split_arg1 = arg0; break;

		case 'w': // managing windows
			// there are ? preconfigured layouts
			// they take arguments on how bit particular panes are
			// xxx how to decide on buffer dimensions? (BUFFER_MULTIPLIER)
			// todo don't clear windows if layout is the same?

			// 2 I (simple) splits
			// 4 T splits
			// 2 H splits
			// 3 X splits

#define SPLIT_HORIZONTAL_MULTIPLIER 4 // smallest font height?
#define SPLIT_VERTICAL_MULTIPLIER 8 // 8 or some font width?

			sw_reset_all();

			switch (arg0) {
				sw *tsw;

			case 0: // no split
				(void)tsw;
				tsw = sw_new(0, 0,
					   DISPLAY_WIDTH, DISPLAY_HEIGHT,
					   DISPLAY_WIDTH * BUFFER_MULTIPLIER, DISPLAY_HEIGHT);
				break;

			case 1: // horizontal split
				// y
				split_arg0 = console_tailor_arg(split_arg0,
												DISPLAY_HEIGHT / 2,
												SPLIT_HORIZONTAL_MULTIPLIER,
												DISPLAY_HEIGHT);

				tsw = sw_new(0, 0,
							 DISPLAY_WIDTH, split_arg0,
							 DISPLAY_WIDTH * BUFFER_MULTIPLIER, split_arg0);

				tsw = sw_new(0, DISPLAY_HEIGHT - split_arg0,
							 DISPLAY_WIDTH, DISPLAY_HEIGHT - split_arg0,
							 DISPLAY_WIDTH * BUFFER_MULTIPLIER, DISPLAY_HEIGHT - split_arg0);
				break;

			case 2: // vertical split
				// x
				split_arg0 = console_tailor_arg(split_arg0,
												DISPLAY_WIDTH / 2,
												SPLIT_VERTICAL_MULTIPLIER,
												DISPLAY_WIDTH);

				tsw = sw_new(0, 0,
							 split_arg0, DISPLAY_HEIGHT,
							 split_arg0 * BUFFER_MULTIPLIER, DISPLAY_HEIGHT);

#ifndef HORIZONTAL_SPLIT_HORZINONTAL_BUFFER
				tsw = sw_new(split_arg0, 0,
							 DISPLAY_WIDTH - split_arg0, DISPLAY_HEIGHT,
							 (DISPLAY_WIDTH - split_arg0) * BUFFER_MULTIPLIER, DISPLAY_HEIGHT);
#else
				tsw = sw_new(split_arg0, 0,
							 DISPLAY_WIDTH - split_arg0, DISPLAY_HEIGHT,
							 // XXX horizontal buffer broken?
							 (DISPLAY_WIDTH - split_arg0), DISPLAY_HEIGHT * BUFFER_MULTIPLIER);
#endif
				break;

			case 3: // T-left split
				break;

			case 4: // T-right split
				// x
				split_arg0 = console_tailor_arg(split_arg0,
												DISPLAY_WIDTH / 2,
												SPLIT_VERTICAL_MULTIPLIER,
												DISPLAY_WIDTH);

				// y
				split_arg1 = console_tailor_arg(split_arg1,
												DISPLAY_HEIGHT / 2,
												SPLIT_HORIZONTAL_MULTIPLIER,
												DISPLAY_HEIGHT);

				tsw = sw_new(0, 0,
							 split_arg0, DISPLAY_HEIGHT,
							 split_arg0 * BUFFER_MULTIPLIER, DISPLAY_HEIGHT);

				tsw = sw_new(split_arg0, 0,
							 DISPLAY_WIDTH - split_arg0, split_arg1,
							 (DISPLAY_WIDTH - split_arg0) * BUFFER_MULTIPLIER, split_arg1);

				tsw = sw_new(split_arg0, split_arg1,
							 DISPLAY_WIDTH - split_arg0, DISPLAY_HEIGHT - split_arg1,
							 (DISPLAY_WIDTH - split_arg0) * BUFFER_MULTIPLIER, DISPLAY_HEIGHT - split_arg1);
				break;

			case 5: // T-down split
				break;

			case 6: // T-up split
				// x
				split_arg0 = console_tailor_arg(split_arg0,
												DISPLAY_WIDTH / 2,
												SPLIT_VERTICAL_MULTIPLIER,
												DISPLAY_WIDTH);

				// y
				split_arg1 = console_tailor_arg(split_arg1,
												DISPLAY_HEIGHT / 2,
												SPLIT_HORIZONTAL_MULTIPLIER,
												DISPLAY_HEIGHT);

				tsw = sw_new(0, 0,
							 split_arg0, split_arg1,
							 split_arg0 * BUFFER_MULTIPLIER, split_arg1);

				tsw = sw_new(split_arg0, 0,
							 DISPLAY_WIDTH - split_arg0, split_arg1,
							 (DISPLAY_WIDTH - split_arg0) * BUFFER_MULTIPLIER, split_arg1);

				tsw = sw_new(0, split_arg1,
							 DISPLAY_WIDTH, DISPLAY_HEIGHT - split_arg1,
							 DISPLAY_WIDTH * BUFFER_MULTIPLIER, DISPLAY_HEIGHT - split_arg1);
				break;
			}

			sw_build_next_window_table();

			FOREACH_WINDOW(csw, cswi) {
				if (cswi == 0)
					continue;

				csw->font = font_get_fitting(font_first, csw->height);
			}

			window_select(0);
			split_arg0 = 0;
			split_arg1 = 0;
			break;

		case 'o':
			window_select(arg0);
			break;
		}
	}

	console_mode = CONSOLE_MODE_DEFAULT;
}

void parse_letter(sw *csw, uint8_t c) {
	static uint8_t wcbytes;
	static uint32_t wc;

	switch (console_mode) {
	case CONSOLE_MODE_ESCAPE_SEQUENCE:
		parse_escape_sequence(csw, c);
		return;
	}

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
		if (csw->flags & FLAG_SCROLL_WHEN_OVERFLOW) {
			if (csw->cursor_x + FONT_WIDTH > csw->width) {
				// don't scroll back every time a new character is added
				if (csw->scroll_mode != SCROLL_LEFT) {
					csw->scroll_mode = SCROLL_LEFT;
					sw_scroll(csw, SCROLL_BUFFER_PRE_START); // won't work at first I think
				}
			} else {
				csw->scroll_mode = NO_SCROLL;
				sw_scroll(csw, SCROLL_BUFFER_START);
			}
		}

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
		console_mode = CONSOLE_MODE_ESCAPE_SEQUENCE;
		console_buffer_pointer = &console_buffer[0];
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

		// cycle through font
	case KEY_CTRL('G'):
		if (csw->font->next->next)
			csw->font = csw->font->next;
		else
			csw->font = font_first;
		sw_reset(csw);
		break;

		// cycle through windows
	case KEY_CTRL('O'):
		// skip root window
		if (main_window + 1 < sw_counter)
			main_window++;
		else
			main_window = 1;
		break;

		// toggle scrolling (and delay)
	case KEY_CTRL('R'):
		sw_scroll(csw, SCROLL_BUFFER_START);
		sw_scroll(csw, SCROLL_BUFFER_START_VERT);
		csw->scroll_mode = (csw->scroll_mode + 1) % 5;
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
	case KEY_CTRL('D'):
		fprintf(&uart, "( %4i, %4i )  < %3i, %3i, %3i > \r",
				csw->offset_x,
				csw->offset_y,
				space_pre,
				space_in,
				space_post
			   );

		switch (c) {
		case KEY_CTRL('S'): csw->scroll_mode = SCROLL_LEFT;  break;
		case KEY_CTRL('T'): csw->scroll_mode = SCROLL_RIGHT; break;
		case KEY_CTRL('U'): csw->scroll_mode = SCROLL_UP;    break;
		case KEY_CTRL('D'): csw->scroll_mode = SCROLL_DOWN;  break;
		}

		sw_scroll_tick();
		csw->scroll_mode = NO_SCROLL;

		break;

	case KEY_CTRL(']'): // group separator
		break;

	case '\f': // ^L
		sw_reset(csw);
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
