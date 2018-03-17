/*
 * before:
 *
 *    ---------------        -----------
 *   |               |      |           |
 *   |         µC    |      |    RAM    |
 *   |               |      |           |
 *   | ABCD  SI  SO  |      |   SI SO   |
 *    ---------------        -----------
 *       |       |
 *       |       |
 *       |       |
 *    ----------------
 *   | ABCD      SI   |
 *   |                |
 *   |       display  |
 *   |                |
 *    ----------------
 *
 *
 * after:
 *
 *    ----------------        -----------
 *   |                |      |           |
 *   |          µC    |      |    RAM    |
 *   |                |      |           |
 *   | ABCD   SI  SO  |      |   SI SO   |
 *    ----------------        -----------
 *       |     |  |              |  |
 *       |     |   --------------   |
 *       |     |                    |
 *       |      --+-----------------
 *       |        |
 *       |        |
 *    -----------------
 *   | ABCD       SI   |
 *   |                 |
 *   |        display  |
 *   |                 |
 *    -----------------
 *
 *
 */

#include "sram.h"

/*
 * render data to ram: sram_write(uint8_t *data, uint16_t data_length)
 *
 * spi_write(
 *
 *
 * scan would look like: sram_read(data_length);
 *
 * spi_write(SRAM_COMMAND_READ);
 * spi_write_double(0); // first buffer
 * while (data_count--)
 * 	spi_read();
 *
 *
 */
