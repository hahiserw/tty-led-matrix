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
sw sw_set[SW_MAX_COUNT];
sw *sw_sorted[SW_MAX_COUNT];

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
	nsw->flags         = 0 | FLAG_SCROLL_WHEN_OVERFLOW;

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

void sw_clear_buffer(sw *csw)
{
	memset(csw->buffer, 0, WINDOW_BUFFER_SIZE(csw));
}

void sw_reset(sw *csw)
{
	sw_clear_buffer(csw);
	// memset(csw, 0, sizeof(sw));

	// TODO console_cursor_move(0, 0);
	csw->cursor_x = 0;
	csw->cursor_y = 0;
}

void sw_reset_all(void)
{
	// for (uint8_t i = sw_counter - 1; i - 1; i--) {
	// 	sw *csw = &sw_set[i];
	// }

	FOREACH_WINDOW(csw, cswi) {
		if (cswi == 0)
			continue;

		sw_reset(csw);
	}

	sw_counter = 1;
}


// global for debugging
sw *sw_next[WINDOW_NEXT_MAX];

// build pointer table by iterating over previous windows and checking if
// they're tanget
int8_t sw_build_next_window_table(void)
{
	// static sw *sw_next[WINDOW_NEXT_MAX];
	// static sw sw_next[SW_MAX_COUNT][SW_LINE_WINDOWS_MAX_COUNT];

	sw **n = sw_next;

	// find tangent windows set window's next pointers table
	//  ----              ----
	// |    |----    ----|    |
	// | cw |    |  |    | dw |
	//  ----| dw |  | cw |    |
	// |    |----    ---------
	// |    |
	//  ----
	FOREACH_WINDOW(csw, cswi) {
		csw->next = n;

		FOREACH_WINDOW(dsw, dswi) {
			// no need to check root_window here
			// nor same window with itself
			if (dsw == root_window || csw == dsw)
				continue;

			// special case
			// if window begins at x == 0 and is just inside root_window
			if (csw == root_window && dsw->x == 0)
				;
			else
				// if not tangent horizontaly
				if (dsw->x != csw->x + csw->width)
					continue;

			if (WINDOWS_HORIZONTALY_TANGENT(csw, dsw)) {
				if ((n - sw_next < WINDOW_NEXT_MAX))
					*n++ = dsw;
				else
					return -1;
			}
		}

		*n++ = NULL;
	}

	return n - sw_next;
}

// x macros?
inline void sw_scroll(sw *csw, scroll_buffer position)
{
	switch (position) {
	case SCROLL_BUFFER_PRE_START:
		csw->offset_x = -csw->width;
		break;

	case SCROLL_BUFFER_PRE_START_VERT:
		csw->offset_y = -(csw->cursor_y + csw->font->height);
		break;

	case SCROLL_BUFFER_DATA_END:
		csw->offset_x = csw->cursor_x;
		break;

	case SCROLL_BUFFER_DATA_END_VERT:
		csw->offset_y = csw->cursor_y + csw->font->height;
		break;

	case SCROLL_BUFFER_START:
		csw->offset_x = 0;
		break;

	case SCROLL_BUFFER_START_VERT:
		csw->offset_y = 0;
		break;
	}
}

#if 0
// todo emit escape sequences when on certain offsets
int8_t sw_scroll_check(sw *csw, scroll_buffer position)
{
	switch (position) {
	case SCROLL_BUFFER_START_VERT:
	case SCROLL_BUFFER_PRE_START_VERT:
	case SCROLL_BUFFER_DATA_END_VERT:
		return 0;

	case SCROLL_BUFFER_START:
		return csw->offset_x == 0;

	case SCROLL_BUFFER_PRE_START:
		return csw->offset_x == -csw->width;

	case SCROLL_BUFFER_DATA_END:
		return csw->offset_x == csw->cursor_x;
	}

	return -1; // XXX
}
// #endif


#define SCROLL_UPDATE(csw, check, axis, change, end_scroll) \
	if (check) { \
		csw->offset_ ## axis += change; \
		/* if (sw_scroll_check(csw, SCROLL_BUFFER_START)) \
			console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_START); */ \
	} else { \
		sw_scroll(csw, end_scroll); \
		/* console_write(CONSOLE_SCROLLING_RIGHT_CYCLE_END); */ \
	}
#endif

#define SCROLL_UPDATE(csw, offset, check, change, end_scroll) \
	if (!(csw->offset check)) \
		sw_scroll(csw, end_scroll); \
	csw->offset change;

inline void sw_scroll_tick(void)
{
	// these are written so there's one empty frame after the end of
	// scrolling cycle
	FOREACH_WINDOW(csw, c) {
		switch (csw->scroll_mode) {
		case NO_SCROLL:
			continue;

		case SCROLL_LEFT:
			SCROLL_UPDATE(csw,
						  offset_x,
						  < csw->cursor_x,
						  ++,
						  SCROLL_BUFFER_PRE_START);
			break;

		case SCROLL_RIGHT:
			SCROLL_UPDATE(csw,
						  offset_x,
						  > -(pos_t)csw->width,
						  --,
						  SCROLL_BUFFER_DATA_END);
			break;

		case SCROLL_UP:
			SCROLL_UPDATE(csw,
						  offset_y,
						  < csw->cursor_y + csw->font->height,
						  ++,
						  SCROLL_BUFFER_PRE_START_VERT);
			break;

		case SCROLL_DOWN:
			SCROLL_UPDATE(csw,
						  offset_y,
						  > -(csw->cursor_y + csw->font->height),
						  --,
						  SCROLL_BUFFER_DATA_END_VERT);
			break;

			// for animated images?
		// case SCROLL_SPRITE:
		// 	csw->offset_x += sprite_width;
		// 	break;
		}

		if (csw->flags & FLAG_REPORT_OVERFLOWS) {
#if 0
			switch (csw->scroll_mode) {
			case SCROLL_LEFT:
			case SCROLL_RIGHT:
				if (csw->offset_x == 0)
					fprintf(&uart, "\e_%ia", sw_get_window_number(csw));
				else if (csw->offset_x == -(pos_t)csw->width)
					fprintf(&uart, "\e_%ib", sw_get_window_number(csw));
				else if (csw->offset_x == csw->cursor_x)
					fprintf(&uart, "\e_%ic", sw_get_window_number(csw));
				break;

			case SCROLL_UP:
			case SCROLL_DOWN:
				if (csw->offset_y == 0)
					fprintf(&uart, "\e_%id", sw_get_window_number(csw));
				break;
			}
#endif
		}
	}
}
