#include "gb_cart.h"
#include "mcp23s17_api.h"
#include <string.h>

// Definiciones de pines para MCP1 (Control de direcciones)
#define MCP1_ADDR_LOW_PORT    MCP23S17_PORT_A  // GPA0-GPA7: A0-A7
#define MCP1_ADDR_HIGH_PORT   MCP23S17_PORT_B  // GPB0-GPB7: A8-A15

// Definiciones de pines para MCP2 (Datos y control)
#define GB_MCP2_DATA_PORT        MCP23S17_PORT_A  // GPA0-GPA7: D0-D7
#define GB_MCP2_DATA_HIGH_PORT   MCP23S17_PORT_B  // GPB0-GPB7: D8-D15

// # GB/GBC
//  ## Pines de control en MCP1 (GPA0-GPA7)
#define GB_MCP1_A0_PIN          0  // GPA0: A0
#define GB_MCP1_A1_PIN          1  // GPA1: A1
#define GB_MCP1_A2_PIN          2  // GPA2: A2
#define GB_MCP1_A3_PIN          3  // GPA3: A3
#define GB_MCP1_A4_PIN          4  // GPA4: A4
#define GB_MCP1_A5_PIN          5  // GPA5: A5
#define GB_MCP1_A6_PIN          6  // GPA6: A6
#define GB_MCP1_A7_PIN          7  // GPA7: A7
// ## Pines de control en MCP1 (GPB0-GPB7)
#define GB_MCP1_A8_PIN          0  // GPB0: A8
#define GB_MCP1_A9_PIN          1  // GPB1: A9
#define GB_MCP1_A10_PIN         2  // GPB2: A10
#define GB_MCP1_A11_PIN         3  // GPB3: A11
#define GB_MCP1_A12_PIN         4  // GPB4: A12
#define GB_MCP1_A13_PIN         5  // GPB5: A13
#define GB_MCP1_A14_PIN         6  // GPB6: A14
#define GB_MCP1_A15_PIN         7  // GPB7: A15


// ## Pines de control en MCP2 (GPA0-GPA7)
#define GB_MCP2_CLK_PIN          0  // GPA0: CLK
#define GB_MCP2_WR_PIN           1  // GPA1: WR
#define GB_MCP2_RD_PIN           2  // GPA2: RD
#define GB_MCP2_CS_PIN           3  // GPA3: CS
#define GB_MCP2_AUDIO_PIN        4  // GPA4: AUDIO
#define GB_MCP2_RST_PIN          5  // GPA5: RESET
#define GB_MCP2_VOLTAGE_PIN      6  // GPA6: VOLTAGE_SELECT
#define GB_MCP2_ACTIVITY_PIN     7  // GPA7: ACTIVITY_LED

// ## Pines de datos en MCP2 (GPB0-GPB7)
#define GB_MCP2_D0_PIN          0  // GPB0: D0
#define GB_MCP2_D1_PIN          1  // GPB1: D1
#define GB_MCP2_D2_PIN          2  // GPB2: D2
#define GB_MCP2_D3_PIN          3  // GPB3: D3
#define GB_MCP2_D4_PIN          4  // GPB4: D4
#define GB_MCP2_D5_PIN          5  // GPB5: D5
#define GB_MCP2_D6_PIN          6  // GPB6: D6
#define GB_MCP2_D7_PIN          7  // GPB7: D7



// # GBA
//  ## Pines de control en MCP1
//  (24 Bit Address Bus)
// ## Pines de control en MCP1 (GPA0-GPA7)
#define GBA_MCP1_A0_PIN          0  // GPA0: A0
#define GBA_MCP1_A1_PIN          1  // GPA1: A1
#define GBA_MCP1_A2_PIN          2  // GPA2: A2
#define GBA_MCP1_A3_PIN          3  // GPA3: A3
#define GBA_MCP1_A4_PIN          4  // GPA4: A4
#define GBA_MCP1_A5_PIN          5  // GPA5: A5
#define GBA_MCP1_A6_PIN          6  // GPA6: A6
#define GBA_MCP1_A7_PIN          7  // GPA7: A7

// ## Pines de control en MCP1 (GPB0-GPB7)
#define GBA_MCP1_A8_PIN          0  // GPB0: A8
#define GBA_MCP1_A9_PIN          1  // GPB1: A9
#define GBA_MCP1_A10_PIN         2  // GPB2: A10
#define GBA_MCP1_A11_PIN         3  // GPB3: A11
#define GBA_MCP1_A12_PIN         4  // GPB4: A12
#define GBA_MCP1_A13_PIN         5  // GPB5: A13
#define GBA_MCP1_A14_PIN         6  // GPB6: A14
#define GBA_MCP1_A15_PIN         7  // GPB7: A15

// ## Pines de control en MCP2 (GPA0-GPA7)
#define GBA_MCP2_CLK_PIN          0  // GPA0: CLK
#define GBA_MCP2_WR_PIN           1  // GPA1: WR
#define GBA_MCP2_RD_PIN           2  // GPA2: RD
#define GBA_MCP2_CS_PIN           3  // GPA3: CS
#define GBA_MCP2_CS2_PIN          4  // GPA4: CS2
#define GBA_MCP2_IRQ_PIN          5  // GPA5: IRQ
#define GBA_MCP2_VOLTAGE_PIN      6  // GPA6: VOLTAGE_SELECT
#define GBA_MCP2_ACTIVITY_PIN     7  // GPA7: ACTIVITY_LED

// ## Pines de datos en MCP2 (GPB0-GPB7)
#define GBA_MCP2_A16_PIN          0  // GPB0: A16
#define GBA_MCP2_A17_PIN          1  // GPB1: A17
#define GBA_MCP2_A18_PIN          2  // GPB2: A18
#define GBA_MCP2_A19_PIN          3  // GPB3: A19
#define GBA_MCP2_A20_PIN          4  // GPB4: A20
#define GBA_MCP2_A21_PIN          5  // GPB5: A21
#define GBA_MCP2_A22_PIN          6  // GPB6: A22
#define GBA_MCP2_A23_PIN          7  // GPB7: A23

//  (16 Bit Data Bus)
// ## Pines de control en MCP1 (GPA0-GPA7)
#define GBA_MCP1_D0_PIN          0  // GPA0: A0
#define GBA_MCP1_D1_PIN          1  // GPA1: A1
#define GBA_MCP1_D2_PIN          2  // GPA2: A2
#define GBA_MCP1_D3_PIN          3  // GPA3: A3
#define GBA_MCP1_D4_PIN          4  // GPA4: A4
#define GBA_MCP1_D5_PIN          5  // GPA5: A5
#define GBA_MCP1_D6_PIN          6  // GPA6: A6
#define GBA_MCP1_D7_PIN          7  // GPA7: A7

// ## Pines de control en MCP1 (GPB0-GPB7)
#define GBA_MCP1_D8_PIN          0  // GPB0: A8
#define GBA_MCP1_D9_PIN          1  // GPB1: A9
#define GBA_MCP1_D10_PIN         2  // GPB2: A10
#define GBA_MCP1_D11_PIN         3  // GPB3: A11
#define GBA_MCP1_D12_PIN         4  // GPB4: A12
#define GBA_MCP1_D13_PIN         5  // GPB5: A13
#define GBA_MCP1_D14_PIN         6  // GPB6: A14
#define GBA_MCP1_D15_PIN         7  // GPB7: A15


// Variables globales para los MCP23S17
static MCP23S17* mcp1 = NULL;
static MCP23S17* mcp2 = NULL;

// Función para inicializar los MCP23S17
bool gb_cart_init(MCP23S17* mcp1_instance, MCP23S17* mcp2_instance) {
    mcp1 = mcp1_instance;
    mcp2 = mcp2_instance;
    
    // Configurar MCP1 para control de direcciones
    mcp23s17_port_mode(mcp1, MCP1_ADDR_LOW_PORT, MCP23S17_PIN_MODE_OUTPUT);
    mcp23s17_port_mode(mcp1, MCP1_ADDR_HIGH_PORT, MCP23S17_PIN_MODE_OUTPUT);
    
    // Configurar MCP2 para datos y señales de control
    mcp23s17_port_mode(mcp2, GB_MCP2_DATA_PORT, MCP23S17_PIN_MODE_INPUT);
    mcp23s17_port_mode(mcp2, GB_MCP2_DATA_HIGH_PORT, MCP23S17_PIN_MODE_INPUT);
    
    // Configurar pines de control en MCP2
    mcp23s17_port_mode(mcp2, GB_MCP2_DATA_PORT, MCP23S17_PIN_MODE_OUTPUT);
    
    // Inicializar señales de control en MCP2
    mcp23s17_digital_write(mcp2, GB_MCP2_RD_PIN, GB_MCP2_DATA_PORT, 1);  // RD
    mcp23s17_digital_write(mcp2, GB_MCP2_WR_PIN, GB_MCP2_DATA_PORT, 0);  // WR
    mcp23s17_digital_write(mcp2, GB_MCP2_CS_PIN, GB_MCP2_DATA_PORT, 0);  // CS
    mcp23s17_digital_write(mcp2, GB_MCP2_CLK_PIN, GB_MCP2_DATA_PORT, 1); // CLK
    
    return true;
}

// Función para establecer la dirección del cartucho
void gb_cart_set_address(uint16_t address) {
    // Establecer dirección en MCP1
    uint8_t addr_low = address & 0xFF;
    uint8_t addr_high = (address >> 8) & 0xFF;
    
    mcp23s17_write_port(mcp1, MCP1_ADDR_LOW_PORT, addr_low);
    mcp23s17_write_port(mcp1, MCP1_ADDR_HIGH_PORT, addr_high);
    
    // Establecer señales de control en MCP2
    mcp23s17_digital_write(mcp2, GB_MCP2_RD_PIN, GB_MCP2_DATA_PORT, 1);  // RD
    mcp23s17_digital_write(mcp2, GB_MCP2_WR_PIN, GB_MCP2_DATA_PORT, 0);  // WR
    mcp23s17_digital_write(mcp2, GB_MCP2_CS_PIN, GB_MCP2_DATA_PORT, 0);  // CS
}

// Función para leer un byte del cartucho
bool gb_cart_read_byte(uint16_t address, uint8_t* value) {
    if (!value) return false;
    
    gb_cart_set_address(address);
    
    // Esperar a que la dirección se estabilice
    // furi_delay_ms(3);
    
    // Activar RD
    mcp23s17_digital_write(mcp2, GB_MCP2_RD_PIN, GB_MCP2_DATA_PORT, 0);  // RD
    
    // Esperar a que los datos se estabilicen
    // furi_delay_ms(3);
    
    // Leer datos de MCP2 (D0-D7 en GPB0-GPB7)
    uint8_t data;
    mcp23s17_read_port(mcp2, GB_MCP2_DATA_HIGH_PORT, &data);
    
    // Desactivar RD
    mcp23s17_digital_write(mcp2, GB_MCP2_RD_PIN, GB_MCP2_DATA_PORT, 1);  // RD
    
    // Esperar a que las señales se estabilicen
    // furi_delay_ms(3);
    
    *value = data;
    return true;
}

// Función para escribir un byte al cartucho
void gb_cart_write_byte(uint16_t address, uint8_t value) {
    gb_cart_set_address(address);
    
    // Escribir datos en MCP2
    mcp23s17_write_port(mcp2, GB_MCP2_DATA_PORT, value);
    mcp23s17_write_port(mcp2, GB_MCP2_DATA_HIGH_PORT, 0);
    
    // Establecer WR
    mcp23s17_digital_write(mcp2, GB_MCP2_WR_PIN, GB_MCP2_DATA_PORT, 1);  // WR
    mcp23s17_digital_write(mcp2, GB_MCP2_WR_PIN, GB_MCP2_DATA_PORT, 0);  // WR
}

// Función para obtener el string del tipo de cartucho
uint8_t gb_cart_get_type_string(char* buffer, uint8_t type) {
    switch(type) {
        case 0x00: strcpy(buffer, "ROM ONLY"); return 8;
        case 0x01: strcpy(buffer, "MBC1"); return 4;
        case 0x02: strcpy(buffer, "MBC1+RAM"); return 8;
        case 0x03: strcpy(buffer, "MBC1+RAM+BATTERY"); return 16;
        case 0x05: strcpy(buffer, "MBC2"); return 4;
        case 0x06: strcpy(buffer, "MBC2+BATTERY"); return 12;
        case 0x08: strcpy(buffer, "ROM+RAM"); return 7;
        case 0x09: strcpy(buffer, "ROM+RAM+BATTERY"); return 15;
        case 0x0B: strcpy(buffer, "MMM01"); return 5;
        case 0x0C: strcpy(buffer, "MMM01+RAM"); return 9;
        case 0x0D: strcpy(buffer, "MMM01+RAM+BATTERY"); return 17;
        case 0x0F: strcpy(buffer, "MBC3+TIMER+BATTERY"); return 17;
        case 0x10: strcpy(buffer, "MBC3+TIMER+RAM+BATTERY"); return 21;
        case 0x11: strcpy(buffer, "MBC3"); return 4;
        case 0x12: strcpy(buffer, "MBC3+RAM"); return 8;
        case 0x13: strcpy(buffer, "MBC3+RAM+BATTERY"); return 16;
        case 0x15: strcpy(buffer, "MBC4"); return 4;
        case 0x16: strcpy(buffer, "MBC4+RAM"); return 8;
        case 0x17: strcpy(buffer, "MBC4+RAM+BATTERY"); return 16;
        case 0x19: strcpy(buffer, "MBC5"); return 4;
        case 0x1A: strcpy(buffer, "MBC5+RAM"); return 8;
        case 0x1B: strcpy(buffer, "MBC5+RAM+BATTERY"); return 16;
        case 0x1C: strcpy(buffer, "MBC5+RUMBLE"); return 11;
        case 0x1D: strcpy(buffer, "MBC5+RUMBLE+RAM"); return 15;
        case 0x1E: strcpy(buffer, "MBC5+RUMBLE+RAM+BATTERY"); return 23;
        case 0xFC: strcpy(buffer, "POCKET CAMERA"); return 12;
        case 0xFD: strcpy(buffer, "BANDAI TAMA5"); return 11;
        case 0xFE: strcpy(buffer, "HuC3"); return 4;
        case 0xFF: strcpy(buffer, "HuC1+RAM+BATTERY"); return 15;
        default: strcpy(buffer, "UNKNOWN"); return 7;
    }
}

// Función para leer múltiples bytes del cartucho
bool gb_cart_read_bytes(uint16_t address, uint8_t* buffer, size_t length) {
    if (!buffer) return false;
    
    for (size_t i = 0; i < length; i++) {
        if (!gb_cart_read_byte(address + i, &buffer[i])) {
            return false;
        }
    }
    return true;
}

// Función principal para leer la información del cartucho
bool gb_cart_read_info(GBCartInfo* info) {
    if (!info) return false;
    
    // Leer los primeros 0x180 bytes del cartucho
    uint8_t startRomBuffer[0x180];
    if (!gb_cart_read_bytes(0x0000, startRomBuffer, sizeof(startRomBuffer))) {
        FURI_LOG_E("GB_CART", "Error al leer el header del cartucho");
        return false;
    }
    
    FURI_LOG_I("GB_CART", "Leído 0x0 - 0x0180");
    
    // Limpiar el título
    memset(info->title, 0, sizeof(info->title));
    
    // Leer el título del cartucho (0x0134 - 0x0143)
    uint8_t titleLength = 0;
    for (uint16_t titleAddress = 0x0134; titleAddress <= 0x0143; titleAddress++) {
        char headerChar = startRomBuffer[titleAddress];

        FURI_LOG_I("GB_CART", "Char: %d", headerChar);
        if ((headerChar >= 0x30 && headerChar <= 0x39) || // 0-9
            (headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
            (headerChar >= 0x61 && headerChar <= 0x7A) || // a-z
            (headerChar >= 0x24 && headerChar <= 0x29) || // #$%&'()
            (headerChar == 0x2D) ||                       // -
            (headerChar == 0x2E) ||                       // .
            (headerChar == 0x5F) ||                       // _
            (headerChar == 0x20)) {                       // Space
            info->title[titleAddress - 0x0134] = headerChar;
            titleLength++;
        }
        // Reemplazar con guión bajo
        else if (headerChar == 0x3A) {
            info->title[titleAddress - 0x0134] = '_';
            titleLength++;
        }
        else {
            info->title[titleAddress - 0x0134] = '\0';
            break;
        }
    }
    
    // Leer el tipo de cartucho (offset 0x147)
    info->cart_type = startRomBuffer[0x147];
    
    // Leer el tamaño de ROM (offset 0x148)
    uint8_t rom_size_code = startRomBuffer[0x148];
    info->rom_size = 32768 << rom_size_code;  // 32KB * 2^rom_size_code
    info->rom_banks = info->rom_size / 32768;  // Cada banco es de 32KB
    
    // Leer el tamaño de RAM (offset 0x149)
    uint8_t ram_size_code = startRomBuffer[0x149];
    switch (ram_size_code) {
        case 0: info->ram_size = 0; break;
        case 1: info->ram_size = 2048; break;    // 2KB
        case 2: info->ram_size = 8192; break;    // 8KB
        case 3: info->ram_size = 32768; break;   // 32KB
        default: info->ram_size = 0; break;
    }
    info->ram_banks = info->ram_size / 8192;  // Cada banco es de 8KB
    
    // Verificar características
    info->has_battery = (info->cart_type == 0x03 || // MBC1+RAM+BATTERY
                        info->cart_type == 0x06 || // MBC2+BATTERY
                        info->cart_type == 0x09 || // ROM+RAM+BATTERY
                        info->cart_type == 0x0D || // MMM01+RAM+BATTERY
                        info->cart_type == 0x0F || // MBC3+TIMER+BATTERY
                        info->cart_type == 0x10 || // MBC3+TIMER+RAM+BATTERY
                        info->cart_type == 0x13 || // MBC3+RAM+BATTERY
                        info->cart_type == 0x17 || // MBC4+RAM+BATTERY
                        info->cart_type == 0x1B || // MBC5+RAM+BATTERY
                        info->cart_type == 0x1E || // MBC5+RUMBLE+RAM+BATTERY
                        info->cart_type == 0xFF);  // HuC1+RAM+BATTERY
    
    // Verificar SGB (offset 0x146)
    info->has_sgb = (startRomBuffer[0x146] == 0x03);
    
    // Calcular checksum
    uint16_t checksum = 0;
    for (int i = 0x134; i <= 0x14C; i++) {
        checksum += startRomBuffer[i];
    }
    info->checksum = checksum & 0xFF;
    
    FURI_LOG_I("GB_CART", "Título: %s", info->title);
    FURI_LOG_I("GB_CART", "Tipo: 0x%02X", info->cart_type);
    FURI_LOG_I("GB_CART", "ROM: %luKB (%d banks)", info->rom_size / 1024, info->rom_banks);
    FURI_LOG_I("GB_CART", "RAM: %luKB (%d banks)", info->ram_size / 1024, info->ram_banks);
    FURI_LOG_I("GB_CART", "Batería: %s", info->has_battery ? "Sí" : "No");
    FURI_LOG_I("GB_CART", "SGB: %s", info->has_sgb ? "Sí" : "No");
    FURI_LOG_I("GB_CART", "Checksum: 0x%02X", info->checksum);
    
    return true;
} 