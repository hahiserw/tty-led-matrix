#ifndef _FONT_H
#define _FONT_H


#include <stdint.h>
#include "magic_macros.h"


typedef uint16_t map_cell_t;

typedef struct {
	map_cell_t start_code;
	map_cell_t count;
} map2_t;


struct _font_t;

struct _font_t {
	uint8_t width;
	uint8_t height;

	map_cell_t index_invalid_char;
	map_cell_t map_elements;
	const map2_t *map;

	const uint8_t *data;
	struct _font_t *next;
};

typedef struct _font_t font_t;


#define FONT_FIRST font0
#define FONT(n) font ## n

#define FONT_CONFIG(...) _FONT_CONFIG(__COUNTER__, __VA_ARGS__)
#define _FONT_CONFIG(n, ...) \
	static font_t ECAT(font, INC(n)); \
/* const static font_t ECAT(font, n) PROGMEM = { __VA_ARGS__, ECAT(&font, INC(n)) }; */ \
	static font_t ECAT(font, n) = { __VA_ARGS__, ECAT(&font, INC(n)) };


map_cell_t font_map(font_t *font, map_cell_t code);

extern font_t *font_first;


#endif
