#ifndef _SCREEN_H
#define _SCREEN_H


#include <avr/io.h>
#include <avr/interrupt.h>
#include "main.h" // xxx or rather config.h?
#include "window.h"


#define buffer2d(csw, y, x) \
	csw->buffer[(y) * (csw)->buffer_width / 8 + (x)]

#define screen2d(csw, y, x) \
	csw->buffer[(y) * (csw)->width / 8 + (x)]


// temp
extern uint16_t space_pre;
extern uint16_t space_in;
extern uint16_t space_post;
extern uint8_t xm;
extern uint8_t counter_overflow;
extern uint8_t counter;

void draw_pixel(sw *csw, upos_t x, upos_t y, uint8_t color);
void empty_scan(void);
void scan(void);


#endif
