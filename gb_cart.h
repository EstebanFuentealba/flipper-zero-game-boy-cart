#ifndef GB_CART_H
#define GB_CART_H

#include <stdint.h>
#include <stdbool.h>
#include "mcp23s17_api.h"

// Estructura para almacenar la información del cartucho
typedef struct {
    char title[17];
    uint8_t checksum;
    bool has_sgb;
    bool is_gbc;
    char serial[4];
    uint8_t ram_banks;
    uint8_t rom_banks;
    uint8_t cart_type;
    bool has_battery;
    uint32_t rom_size;
    uint32_t ram_size;
} GBCartInfo;

// Constantes para el cartucho
#define GB_CART_TITLE_START 0x134
#define GB_CART_TITLE_END 0x143
#define GB_CART_SGB_FLAG 0x146
#define GB_CART_CART_TYPE 0x147
#define GB_CART_ROM_SIZE 0x148
#define GB_CART_RAM_SIZE 0x149
#define GB_CART_DEST_CODE 0x14A
#define GB_CART_LICENSE_CODE 0x14B
#define GB_CART_VERSION 0x14C
#define GB_CART_CHECKSUM 0x14E
#define GB_CART_GLOBAL_CHECKSUM 0x14E

// Tipos de cartucho
#define GB_CART_TYPE_ROM_ONLY 0x00
#define GB_CART_TYPE_MBC1 0x01
#define GB_CART_TYPE_MBC1_RAM 0x02
#define GB_CART_TYPE_MBC1_RAM_BATTERY 0x03
#define GB_CART_TYPE_MBC2 0x05
#define GB_CART_TYPE_MBC2_BATTERY 0x06
#define GB_CART_TYPE_ROM_RAM 0x08
#define GB_CART_TYPE_ROM_RAM_BATTERY 0x09
#define GB_CART_TYPE_MMM01 0x0B
#define GB_CART_TYPE_MMM01_RAM 0x0C
#define GB_CART_TYPE_MMM01_RAM_BATTERY 0x0D
#define GB_CART_TYPE_MBC3_TIMER_BATTERY 0x0F
#define GB_CART_TYPE_MBC3_TIMER_RAM_BATTERY 0x10
#define GB_CART_TYPE_MBC3 0x11
#define GB_CART_TYPE_MBC3_RAM 0x12
#define GB_CART_TYPE_MBC3_RAM_BATTERY 0x13
#define GB_CART_TYPE_MBC5 0x19
#define GB_CART_TYPE_MBC5_RAM 0x1A
#define GB_CART_TYPE_MBC5_RAM_BATTERY 0x1B
#define GB_CART_TYPE_MBC5_RUMBLE 0x1C
#define GB_CART_TYPE_MBC5_RUMBLE_RAM 0x1D
#define GB_CART_TYPE_MBC5_RUMBLE_RAM_BATTERY 0x1E

// Funciones para leer el cartucho
bool gb_cart_init(MCP23S17* mcp1, MCP23S17* mcp2);
bool gb_cart_read_info(GBCartInfo* info);
bool gb_cart_read_byte(uint16_t address, uint8_t* value);
void gb_cart_write_byte(uint16_t address, uint8_t value);
void gb_cart_set_address(uint16_t address);
uint8_t gb_cart_get_type_string(char* buffer, uint8_t type);

#endif // GB_CART_H 