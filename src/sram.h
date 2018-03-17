#ifndef _SRAM_H
#define _SRAM_H

#define SRAM_COMMAND_READ   0x03
#define SRAM_COMMAND_WRITE  0x02
#define SRAM_COMMAND_EDIO   0x3b
#define SRAM_COMMAND_RSTIO  0xff
#define SRAM_COMMAND_RDMR   0x05
#define SRAM_COMMAND_WRMR   0x01

#define SRAM_REGISTER_BYTE_MODE        0x00
#define SRAM_REGISTER_PAGE_MODE        0x01
#define SRAM_REGISTER_SEQUENTIAL_MODE  0x02

#endif
