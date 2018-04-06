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

// https://forum.pjrc.com/threads/27000-Teensy-extra-RAM-options
// https://www.parallax.com/sites/default/files/downloads/AN012-SRAM-v1.0.pdf

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
