#ifndef _MAIN_H
#define _MAIN_H


#include <stdio.h>
#include "window.h"

#define NEW_SPICY_SCROLLING


#define DISPLAY_MODULE_WIDTH   64
#define DISPLAY_MODULE_HEIGHT  16
#define DISPLAY_MODULES        4

#define DISPLAY_WIDTH          (DISPLAY_MODULE_WIDTH * DISPLAY_MODULES)
#define DISPLAY_WIDTH_BYTES    (DISPLAY_WIDTH / 8)
#define DISPLAY_HEIGHT         DISPLAY_MODULE_HEIGHT
#define DISPLAY_SIZE           (DISPLAY_WIDTH_BYTES * DISPLAY_HEIGHT)


#define SPI_IFACE

#define FONT_UTF8


#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

/*                       HUB08
 *
 *                   GND  o o  A         D0
 *                   GND  o o  B         D1
 *                   GND  o o  C         D2
 *    PWM...F0        OE  o o  D         D3
 *    (SPI) B2        R1  o o  G1  N/C
 *               N/C  R2  o o  G2  N/C
 *                   GND  o o  LAT       F1
 *                   GND  o o  CLK       B1 (SPI)
 */

// #define PIN_ROW(v) PORTD = v
#define PIN_ROW(v) PORTD = (PORTD & 0xf0) | (v & 0x0f)
#define PIN_A(v)   if (v) PORTD |= 1 << 0; else PORTD &= ~(1 << 0)
#define PIN_B(v)   if (v) PORTD |= 1 << 1; else PORTD &= ~(1 << 1)
#define PIN_C(v)   if (v) PORTD |= 1 << 2; else PORTD &= ~(1 << 2)
#define PIN_D(v)   if (v) PORTD |= 1 << 3; else PORTD &= ~(1 << 3)

// xxx or make oe pwm // PB5 - OC1A
// #define PIN_EO(v) (TCCR1B = v? 0x01: 0)
#define PIN_OE_TGL  PINF = 1 << 0
#define PIN_LAT_TGL PINF = 1 << 1
#define PIN_OE(v)  if (v) PORTF |= 1 << 0; else PORTF &= ~(1 << 0)
#define PIN_LAT(v) if (v) PORTF |= 1 << 1; else PORTF &= ~(1 << 1)

// #ifndef SPI_IFACE
#define PIN_CLK(v) if (v) PORTB |= 1 << 1; else PORTB &= ~(1 << 1)
#define PIN_R1(v)  if (v) PORTB |= 1 << 2; else PORTB &= ~(1 << 2)
// #endif

#define NOP asm("nop")

// TODO mask to invert?
#define spi_write(v) \
			do { \
				SPDR = (v); \
				while (!(SPSR & (1 << SPIF))) \
				; \
			} while(0)


extern uint8_t main_buffer[];
extern FILE uart;
extern sw *root_window;

extern uint8_t main_window;

void die(void);

#endif
