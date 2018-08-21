#ifndef _WINDOW_H
#define _WINDOW_H


#include "font.h"


#define SW_MAX_COUNT 6
#define SW_LINE_WINDOWS_MAX_COUNT 4

#define WINDOW_NEXT_MAX 10


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
	SCROLL_BUFFER_DATA_END,
	SCROLL_BUFFER_START_VERT,
	SCROLL_BUFFER_PRE_START_VERT,
	SCROLL_BUFFER_DATA_END_VERT,
} scroll_buffer; // todo rename to scroll_position?

struct _sw;
typedef struct _sw sw;
struct _sw {
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
	sw **next; // following windows (on screen)

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
};


extern sw sw_set[SW_MAX_COUNT];
extern sw *sw_sorted[SW_MAX_COUNT];
extern uint8_t sw_counter;

extern sw *sw_next[10];

sw *sw_new(upos_t, upos_t, upos_t, upos_t, upos_t, upos_t, scroll, font_t *font);
void sw_clear_buffer(sw *csw);
void sw_reset(sw *csw);
void sw_reset_all(void);
int8_t sw_build_next_window_table(void);

void sw_scroll(sw *csw, scroll_buffer position);
// int8_t sw_scroll_check(sw *csw, scroll_buffer position);
void sw_scroll_tick(void);

static inline sw *get_next_window(sw *csw, upos_t y)
{
	sw *nsw = *csw->next;

	for (uint8_t i = 0; i < WINDOW_NEXT_MAX; i++) {
		if (!nsw)
			break;

		// better to check ending range first - less comparisons
		if (y < nsw->y + nsw->height && y >= nsw->y)
			return nsw;

		nsw++;
	}

	return NULL;
}

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

#define IN_RANGE(v, a, b) \
	((v) >= (a) && (v) <= (b))

#define WINDOWS_HORIZONTALY_TANGENT(csw, dsw) \
	( \
	 (IN_RANGE(csw->y, dsw->y, dsw->y + dsw->height) \
	  && IN_RANGE(dsw->y + dsw->height, csw->y, csw->y + csw->height)) \
	  || \
	  (IN_RANGE(dsw->y, csw->y, csw->y + csw->height) \
	   && IN_RANGE(csw->y + csw->height, dsw->y, dsw->y + dsw->height)))


#endif
