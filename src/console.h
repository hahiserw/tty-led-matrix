#ifndef _CONSOLE_H
#define _CONSOLE_H


#include "window.h"


#define CONSOLE_TAB_SIZE 8

#define KEY_CTRL(v) \
	(((v) < 'a')? \
	((v) - 'A' + 0x01): \
	((v) - 'a' + 0x01))


void parse_letter(sw *csw, uint8_t c);
void parse_new_line(sw *csw);

void draw_text(sw *csw, char *m);

#ifdef FONT_UTF8
void draw_letter(sw *, uint32_t);
#else
void draw_letter(sw *, uint8_t);
#endif


void console_write(char *);
// #define CONSOLE_SCROLLING_UP_CYCLE_END     "\e[A"
// #define CONSOLE_SCROLLING_DOWN_CYCLE_END   "\e[B"
// #define CONSOLE_SCROLLING_RIGHT_CYCLE_END  "\e[C"
// #define CONSOLE_SCROLLING_LEFT_CYCLE_END   "\e[D"
// for now
#define CONSOLE_SCROLLING_UP_CYCLE_START     "a"
#define CONSOLE_SCROLLING_DOWN_CYCLE_START   "b"
#define CONSOLE_SCROLLING_RIGHT_CYCLE_START  "c"
#define CONSOLE_SCROLLING_LEFT_CYCLE_START   "d"
#define CONSOLE_SCROLLING_UP_CYCLE_END       "A"
#define CONSOLE_SCROLLING_DOWN_CYCLE_END     "B"
#define CONSOLE_SCROLLING_RIGHT_CYCLE_END    "C"
#define CONSOLE_SCROLLING_LEFT_CYCLE_END     "D"

#endif
