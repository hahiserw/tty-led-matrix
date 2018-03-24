#ifndef _FONT_H
#define _FONT_H


#include "magic_macros.h"


struct _font_t;

struct _font_t {
	uint8_t width;
	uint8_t height;
	uint32_t (*map)(uint32_t);
	const uint8_t *data;
	struct _font_t *next;
};

typedef struct _font_t font_t;


#define FONT_FIRST font0

#define FONT_CONFIG(...) _FONT_CONFIG(__COUNTER__, __VA_ARGS__)
#define _FONT_CONFIG(n, ...) \
	static font_t ECAT(font, INC(n)); \
/* const static font_t ECAT(font, n) PROGMEM = { __VA_ARGS__, ECAT(&font, INC(n)) }; */ \
	static font_t ECAT(font, n) = { __VA_ARGS__, ECAT(&font, INC(n)) };


// uint8_t font_count(void)
// {
// 	uint8_t count = 0;
// 	font_t *cf = &FONT_FIRST;
//
// 	while (cf->data) {
// 		count++;
// 		cf = cf->next;
// 	}
// }


#endif
