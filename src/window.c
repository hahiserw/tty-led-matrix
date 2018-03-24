/*
 * Here I bestow you with a gift of srcollable windows.
 * I fact a nuber of them. So pay attention as I won't repeat myself.
 *
 * There's only one buffer, that contains data to display - by design not
 * movable, just updatable.
 *
 * A number of srcollabel windows of different sizes and with different
 * positions can be added.
 *
 * Each of them has the following fields: see .h file
 *
 * ,----------------------------------------.
 * |a            |b         |c              |
 * |             |          |               |
 * |- - - - - - -+- - - - - +               |
 * |d                       |               |
 * |- - - - - - - - - -+- - + - - - - - - - |
 * |e                  |f                   |
 * |                   |                    |
 * `----------------------------------------'
 *
 *     x   y   w   h
 * a ( 0,  0, 14,  3)
 * b (15,  0, 11,  3)
 * c (26,  0, 16,  5)
 * d ( 0,  4, 25,  2)
 * e ( 0,  6, 20,  2)
 * f (21,  6, 20,  2)
 *
 * todo window in window?
 */

#include <stdlib.h>
#include <stdint.h>

#include "main.h"
#include "window.h"
#include "console.h"
#include "font.h"


// 2.5KB RAM

// sw buffor size will be 4 * width for sideway scrolling, 8 * height otherwise?
// cuz letter are taller than wider
// or no, it should depend on the display modules configuration

// sw *sw_sorted[DISPLAY_HEIGHT][SW_LINE_WINDOWS_MAX_COUNT]; // or linked lists?

#if 0
void sw_display(void) {
	// 1. sort windows by y and then by x
	// 1. for every y render given windows

	sw *csw = sw_sorted[0][0];

	for (upos_t y = 0; y < DISPLAY_HEIGHT; y += csw->y + csw->height) {
		// sw_set_local = sw_y_sorted[y];
		// foreach(csw) {
		//  x = 0;
		//  for (; x < csw->x + csw->width; x++)
		//    SPDR = csw->buffer + csw->offset_x;
	}

	uint8_t y, xb, *d, SPDR;
	// foreach y
	// foreach xb {

	sw *csw = &sw_set[0];

	if (y == csw->y)
		d = csw->buffer + y * csw->width + csw->offset_x;

	SPDR = *d;

	// }
}
#endif

uint8_t sw_counter;
sw sw_set[SW_MAX_COUNT + 1]; // the last one should be null, so foreach would work
sw *sw_sorted[SW_MAX_COUNT + 1];

// void sw_clear(sw *csw)
// {
// }

sw *sw_new(upos_t x, upos_t y,
		   upos_t width, upos_t height,
		   upos_t buffer_width, upos_t buffer_height,
		   scroll scroll_mode,
		   font_t *font)
{
	if (sw_counter + 1 >= SW_MAX_COUNT)
		return NULL;

	sw *nsw = &sw_set[sw_counter];

	if (width == 0 || height == 0
		|| x + width > DISPLAY_WIDTH || y + height > DISPLAY_HEIGHT)
		return NULL;

	// TODO also check if there's enough of buffer left

	nsw->x             = x;
	nsw->y             = y;
	nsw->width         = width;
	nsw->height        = height;
	nsw->buffer_width  = buffer_width; // *2 for scrolling
	nsw->buffer_height = buffer_height;

	nsw->offset_x      = 0;
	nsw->offset_y      = 0;
	nsw->cursor_x      = 0;
	nsw->cursor_y      = 0;

	nsw->scroll_mode   = scroll_mode;
	nsw->font          = font;

	// nope...
	// nsw->buffer = main_buffer + sw_counter * buffer_width * buffer_height / 8;
	// nsw->buffer = main_buffer_end;
	// main_buffer_end += dsw->buffer_width * dsw->buffer_height / 8;

	if (sw_counter)
		nsw->buffer = sw_set[sw_counter - 1].buffer_end;
	else
		nsw->buffer = main_buffer;

	// xxx needs to be ceil div
	nsw->buffer_end = nsw->buffer + buffer_width * buffer_height / 8;

	sw_counter++;

	return nsw;
}

void sw_del(sw *dsw)
{
	// main_buffer_end -= dsw->buffer_width * dsw->buffer_height / 8;
	dsw->width = 0;
}

void sw_scroll(sw *csw, scroll_buffer position)
{
	switch (position) {
	case SCROLL_BUFFER_START:
#ifdef NEW_SPICY_SCROLLING
		csw->offset_x = 0;
#else
		csw->offset_x = csw->buffer_width + csw->width;
#endif
		break;

	case SCROLL_BUFFER_PRE_START:
#ifdef NEW_SPICY_SCROLLING
		csw->offset_x = -csw->width;
#else
		csw->offset_x = csw->buffer_width;
#endif
		break;
	}
}

uint8_t sw_scroll_check(sw *csw, scroll_buffer position)
{
	switch (position) {
	case SCROLL_BUFFER_START:
#ifdef NEW_SPICY_SCROLLING
		return csw->offset_x == 0;
#else
		return csw->offset_x == csw->buffer_width + csw->width;
#endif

	case SCROLL_BUFFER_PRE_START:
#ifdef NEW_SPICY_SCROLLING
		return csw->offset_x == -csw->width;
#else
		return csw->offset_x == csw->buffer_width;
#endif
	}

	return 0; // or 255 or sth?
}


#define SCROLL_UPDATE(csw, check, axis, change) \
	if (check) { \
		csw->offset_ ## axis += change; \
		if (sw_scroll_check(csw, SCROLL_BUFFER_START)) \
			console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_START); \
	} else { \
		sw_scroll(csw, SCROLL_BUFFER_PRE_START); \
		console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_END); \
	}

inline void sw_scroll_tick(void)
{
	FOREACH_WINDOW(csw, c) {
		// if (csw->buffer == NULL) // means it's not active?
		// 	continue;

		switch (csw->scroll_mode) {
		case NO_SCROLL:
			break;

		case SCROLL_LEFT:
			SCROLL_UPDATE(csw,
						  csw->offset_x + 1 < csw->cursor_x,
						  // || csw->offset_ ## axis < csw->buffer_width)
						  x,
						  1);
			break;

		case SCROLL_RIGHT:
			SCROLL_UPDATE(csw,
						  csw->offset_x - 1 > -csw->width,
						  x,
						  -1);
		
			break;

		case SCROLL_LEFT + 10:
#ifdef NEW_SPICY_SCROLLING
			if (csw->offset_x + 1 < csw->cursor_x) { // || < csw->buffer_width for safety
				// if (csw->offset_x < csw->cursor_x) // should be this
				csw->offset_x++;

				if (sw_scroll_check(csw, SCROLL_BUFFER_START))
					console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_START);
			} else {
				sw_scroll(csw, SCROLL_BUFFER_PRE_START);
				console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_END);
			}
#else
			// XXX breaks if cursor_x / FONT_WIDTH > 96
			if (csw->offset_x < csw->buffer_width + csw->width + csw->cursor_x) {
				csw->offset_x++; // % some_width?
				// csw->offset_x = (csw->offset_x + 1) % 2048; // % some_width?

				if (sw_scroll_check(csw, SCROLL_BUFFER_START))
					// XXX about 30 cycles late?
					console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_START);
			} else {
				// csw->offset_x = csw->buffer_width;
				sw_scroll(csw, SCROLL_BUFFER_PRE_START);

				// shouldn't be here?
				console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_END);
				
				// automatically clear the screen after 3 cycles have passed?
			}
#endif
			break;

		case SCROLL_RIGHT + 10:
			csw->offset_x = (csw->offset_x - 1) % (csw->buffer_width * 2);
			// csw->offset_x = (csw->offset_x - 1) % (csw->cursor_x * 2);
			break;

		case SCROLL_UP:
			csw->offset_y = (csw->offset_y + 1) % (csw->buffer_height * 2);
			break;

		case SCROLL_DOWN:
			csw->offset_y = (csw->offset_y - 1) % (csw->buffer_height * 2);
			break;

			// for animated images?
		// case SCROLL_SPRITE:
		// 	csw->offset_x += sprite_width;
		// 	break;
		}
	}
}

inline sw *sw_get_by_y(upos_t y)
{
	FOREACH_WINDOW(csw, c) {
		if (y >= csw->y && y < csw->y + csw->height)
			// for (uint8_t j = 0; j < SW_LINE_WINDOWS_MAX_COUNT; j++)
			// 	if (x >= csw->x && x <= csw->x + csw->width)
			// 	and call it every pixel lol?
			return csw;
	}

	return NULL;
}

