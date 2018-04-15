#include "screen.h"
#include "window.h"


static uint8_t y;


#define COUNTER_BOUND 5
uint8_t counter = 0;
uint8_t counter_overflow = COUNTER_BOUND - 1; // initial scroll speed


void draw_pixel(sw *csw, upos_t x, upos_t y, uint8_t color) {
	if (x > csw->buffer_width || y > csw->buffer_height)
		return;

	// a nice 2d pointer :D
	// uint8_t (*buffer2d)[csw->width / 8][csw->height] =
	// 	(uint8_t ***)csw->buffer;

	if (color)
		buffer2d(csw, y, x / 8) |=   0x80 >> (x & 0x07);
	else
		buffer2d(csw, y, x / 8) &= ~(0x80 >> (x & 0x07));
}


void empty_scan(void)
{
	// empty frame (for lower power usage / brighness control)
	// empty line actually
	PIN_OE_TGL;
	PIN_ROW(y);
	PIN_LAT_TGL;
	PIN_LAT_TGL;
	// (8 bytes per line = 64 pixels) * DISPLAY_MODULES
	for (uint8_t xb = 0; xb < DISPLAY_WIDTH_BYTES; xb++)
		spi_write(0);
	PIN_OE_TGL;

	// y = (y + 1) & 0x0f;
}


// temp for debugging
uint16_t space_pre;
uint16_t space_in;
uint16_t space_post;
uint8_t xm;

static inline void sub_scan(sw *csw, upos_t gy)
{
	pos_t offset_y = csw->offset_y;
	upos_t width   = csw->width;
	upos_t height  = csw->height;
	upos_t y       = csw->y;

	if (gy + offset_y - y < 0 || gy + offset_y - y >= height) {
		for (uint8_t s = 0; s < width / 8; s++)
			spi_write(0);

		return;
	}

	uint8_t *dy = &buffer2d(csw, gy + offset_y - y, 0);
	uint8_t *d;

	upos_t buffer_width = csw->buffer_width;
	pos_t offset_x      = csw->offset_x;

	// if (csw->scroll_mode != NO_SCROLL #<{(| offset_x_changed()|)}>#) {
	if (1) {
		xm = offset_x & 0x07;

		// ox = -sw .. bw
		// possibilities (. - buffer data, ^ - screen)
		if (offset_x >= 0) {
			// ox >= 0
			// pre = 0
			// in  = 0
			space_pre = 0;

			if (offset_x > buffer_width - width) {
			// if: ox + sw > bw
			// in = 0 .. sw
			//    = bw - ox + sw
			//   ...........................
			//                            ^^^^
				space_post = (width - (buffer_width - offset_x)) / 8;
			// sw - (bw - ox)
			} else {
			//                            ...........................
			//                            ^^^^
			//                    ...........................
			//                            ^^^^
				space_post = 0;
			}

			space_in = width / 8 - space_post;
		} else {
			// ox < 0
			// pre = -ox
			// in  = sw - ox
			//                                ...........................
			//                            ^^^^
			//                             ...........................
			//                            ^^^^
			space_pre  = -offset_x / 8 + (xm? 1: 0) - 1; // hmm
			space_in   = width / 8 - space_pre;
			space_post = 0;
		}
	}

	if (offset_x > 0)
		d = dy + offset_x / 8;
	else
		d = dy;

	for (uint8_t s = 0; s < space_pre; s++) {
		spi_write(0);
	}

	if (offset_x >= 0) {
		for (uint8_t s = 1; s < space_in; s++) {
			spi_write((*d << xm) | (*(d+1) >> (8 - xm)));
			d++;
		}

		if (offset_y > buffer_width - width)
			spi_write(*d << xm);
		else
			spi_write((*d << xm) | (*(d+1) >> (8 - xm)));
	} else {
		spi_write(*d >> (8 - xm));

		for (uint8_t s = 1; s < space_in; s++) {
			spi_write((*d << xm) | (*(d+1) >> (8 - xm)));
			d++;
		}
	}

	for (uint8_t s = 0; s < space_post; s++) {
		spi_write(0);
	}

	// should be:
	// - empty pixels before data
	// - data
	// - empty pixels before showing data again?
}

void scan(void)
{
	// why here?! it's delayed by line if it's at the end of this fn
	y = (y + 1) & 0x0f; // from 0 to 15

	cli();

	PIN_ROW(y);

	PIN_LAT(1);
	PIN_LAT(0);

	PIN_OE(0);

	sw *csw = root_window;

	// create some lookup table instead of dynamicaly finding it every frame?
#define _ 1
	// sad for :(
	for (;_;) {
#undef _
		csw = get_next_window(csw, y);

		if (!csw)
			break;

		sub_scan(csw, y);
	}

	PIN_OE(1);

	sei();

	// todo move somewhere?
	// scroll by one only when full screen has been displayed, so there won't
	// be any screen tearing
	if (y == DISPLAY_HEIGHT - 1) {
		if (counter_overflow >= COUNTER_BOUND) {
			if (counter++ == counter_overflow - COUNTER_BOUND + 1) {
				sw_scroll_tick();
				counter = 0;
			}
		} else {
			for (counter = 0; counter < COUNTER_BOUND - counter_overflow; counter++) {
				sw_scroll_tick();
			}
		}
	}
}
