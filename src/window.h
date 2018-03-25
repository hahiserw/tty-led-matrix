#ifndef _WINDOW_H
#define _WINDOW_H


#include "font.h"


#define SW_MAX_COUNT 4
#define SW_LINE_WINDOWS_MAX_COUNT 4

// syn keyword Type pos_t upos_t scroll scroll_buffer

typedef int16_t pos_t;
typedef uint16_t upos_t;

typedef enum {
	NO_SCROLL = 0,
	SCROLL_LEFT,
	SCROLL_RIGHT,
	SCROLL_UP,
	SCROLL_DOWN,
} scroll;

typedef enum {
	SCROLL_BUFFER_START,
	SCROLL_BUFFER_PRE_START,
} scroll_buffer; // todo rename to scroll_position?

typedef struct {
	// screen
	upos_t x;
	upos_t y;
	upos_t width;
	upos_t height;

	// window
	upos_t buffer_width;
	upos_t buffer_height;
	pos_t offset_x;
	pos_t offset_y;

	// todo put into console struct
	// typedef struct {
	pos_t cursor_x;
	pos_t cursor_y;
	// typedef uint8_t font_t;
	font_t *font;
	// } console;

	scroll scroll_mode;

	uint8_t *buffer;
	uint8_t *buffer_end;
} sw;


extern sw sw_set[SW_MAX_COUNT + 1];
extern sw *sw_sorted[SW_MAX_COUNT + 1];
extern uint8_t sw_counter;

sw *sw_new(upos_t, upos_t, upos_t, upos_t, upos_t, upos_t, scroll, font_t *font);

void sw_scroll(sw *csw, scroll_buffer position);
uint8_t sw_scroll_check(sw *csw, scroll_buffer position);
void sw_scroll_tick(void);

sw *sw_get_by_y(upos_t y);


// #define NEW_FOREACH

#ifdef NEW_FOREACH

#define FOREACH_WINDOW(isw, iter) \
	for (sw *isw = &sw_set[0]; isw->buffer != NULL; isw++)

#else

// a trick to init 2 vars of different types in "one" for loop
#define FOREACH_WINDOW(isw, iter) \
	for (uint8_t iter = 0; iter < sw_counter;) \
		for (sw *isw; \
			 iter < sw_counter && (isw = &sw_set[iter]); \
			 iter++) \

#endif


#endif
