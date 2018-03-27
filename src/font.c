#include <avr/pgmspace.h>

#include "font.h"


map_cell_t font_map(font_t *font, map_cell_t code)
{
	map_cell_t i;

	i = font->map3_elements - 1;
	const map3_t *cm3 = &font->map3[0];

	while (i--) {
		map_cell_t rf = pgm_read_word(&cm3->range_from);
		map_cell_t rt = pgm_read_word(&cm3->range_to);

		// first check the ending range so the failing second condition
		// wouldn't have to be checked as well
		if (code <= rt && code >= rf)
			return code + pgm_read_word(&cm3->offset) - rf;

		cm3++;
	}

	i = font->map2_elements - 1;
	const map2_t *cm2 = &font->map2[0];

	while (i--) {
		map_cell_t rf = pgm_read_word(&cm2->range_from);

		if (code == rf)
			return code + pgm_read_word(&cm2->offset) - rf;

		cm2++;
	}

	return font->map3[font->map3_elements - 1].offset;
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
