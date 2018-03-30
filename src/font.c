#include <avr/pgmspace.h>

#include "font.h"

/* examples for
 * {     0,     1 },
 * {    32,    95 },
 * {   160,   224 },
 * {   399,     1 },
 * {   402,     1 },
 * {   416,     2 },
 * {   431,     2 },
 *
 * code == 417
 * 417 < 0 + 1
 * 417 < 32 + 95
 * 417 < 160 + 224
 * 417 < 399 + 1
 * 417 < 402 + 1
 * 417 < 416 + 2
 * 417 >= 416
 * return 417 - 416 + (1 + 95 + 224 + 1 + 1 + 2)
 *
 * code == 5
 * 5 < 0 + 1
 * 5 < 32 + 95
 * 5 >= 32
 * return offset to invalid char
 */

/**
 * @description font index lookup function
 * @param font_t *font  pointer to the font
 * @param map_cell_t code  character's code number
 * @return map_cell_t  start index of glyph's data
 */
map_cell_t font_map(font_t *font, map_cell_t code)
{
	map_cell_t i = font->map_elements - 1;
	const map2_t *cm = &font->map[0];

	map_cell_t offset = 0;

	while (i--) {
		map_cell_t start_code = pgm_read_word(&cm->start_code);
		map_cell_t count      = pgm_read_word(&cm->count);

		// will fail only one check each non matching code
		if (code < start_code + count) {
			if (code >= start_code)
				return code - start_code + offset;
			else
				goto font_search_end;
		}

		offset += count;
		cm++;
	}

font_search_end:
	return font->index_invalid_char;
}

// uint8_t fonts_count(void)
// {
// 	uint8_t count = 0;
// 	font_t *cf = &FONT_FIRST;
// 	font_t *cf = &FONT(0);
//
// 	while (cf->data) {
// 		count++;
// 		cf = cf->next;
// 	}
//
// 	return count;
// }
