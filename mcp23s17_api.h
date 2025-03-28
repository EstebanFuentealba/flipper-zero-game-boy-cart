#ifndef MCP23S17_API_H
#define MCP23S17_API_H

#include <furi.h>
#include <furi_hal_spi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_power.h>

// Definición de registros MCP23S17 (igual que MCP23017)
#define MCP23S17_IODIRA   0x00
#define MCP23S17_IODIRB   0x01
#define MCP23S17_IPOLA    0x02
#define MCP23S17_IPOLB    0x03
#define MCP23S17_GPINTENA 0x04
#define MCP23S17_GPINTENB 0x05
#define MCP23S17_DEFVALA  0x06
#define MCP23S17_DEFVALB  0x07
#define MCP23S17_INTCONA  0x08
#define MCP23S17_INTCONB  0x09
#define MCP23S17_IOCONA   0x0A
#define MCP23S17_IOCONB   0x0B
#define MCP23S17_GPPUA    0x0C
#define MCP23S17_GPPUB    0x0D
#define MCP23S17_INTFA    0x0E
#define MCP23S17_INTFB    0x0F
#define MCP23S17_INTCAPA  0x10
#define MCP23S17_INTCAPB  0x11
#define MCP23S17_GPIOA    0x12
#define MCP23S17_GPIOB    0x13
#define MCP23S17_OLATA    0x14
#define MCP23S17_OLATB    0x15

// Flags para IOCON
#define IOCON_BANK   0x80
#define IOCON_MIRROR 0x40
#define IOCON_SEQOP  0x20
#define IOCON_DISSLW 0x10
#define IOCON_HAEN   0x08
#define IOCON_ODR    0x04
#define IOCON_INTPOL 0x02
#define IOCON_UNUSED 0x01

// Definición de pines
#define MCP23S17_PIN0  0x01
#define MCP23S17_PIN1  0x02
#define MCP23S17_PIN2  0x04
#define MCP23S17_PIN3  0x08
#define MCP23S17_PIN4  0x10
#define MCP23S17_PIN5  0x20
#define MCP23S17_PIN6  0x40
#define MCP23S17_PIN7  0x80

// Here we have things for the SPI bus configuration
#define CLOCK_DIVIDER (2)           // SPI bus speed to be 1/2 of the processor clock speed - 8MHz on most Arduinos

// Control byte and configuration register information - Control Byte: "0100 A2 A1 A0 R/W" -- W=0
#define OPCODEW       (0b01000000)  // Opcode for MCP23S17 with LSB (bit0) set to write (0), address OR'd in later, bits 1-3
#define OPCODER       (0b01000001)  // Opcode for MCP23S17 with LSB (bit0) set to read (1), address OR'd in later, bits 1-3
#define ADDR_ENABLE   (0b00001000)  // Configuration register for MCP23S17, the only thing we change is enabling hardware addressing

// Opcode para SPI
#define MCP23S17_WRITE_OPCODE 0x40
#define MCP23S17_READ_OPCODE  0x41

typedef enum {
    MCP23S17_PORT_A = 0,
    MCP23S17_PORT_B = 1
} MCP23S17Port;

typedef enum {
    MCP23S17_PIN_MODE_OUTPUT = 0,
    MCP23S17_PIN_MODE_INPUT = 1,
    MCP23S17_PIN_MODE_INPUT_PULLUP = 2
} MCP23S17PinMode;

typedef struct {
    uint8_t address;      // Dirección SPI del dispositivo (0-7)
    uint8_t reg_cache[22]; // Cache de valores de registros
    bool initialized;     // Estado de inicialización
    FuriHalSpiBusHandle* spi; // Handle SPI
    const GpioPin* cs_pin;   // Pin CS
} MCP23S17;

// Declaraciones de funciones
bool mcp23s17_spi_write(MCP23S17* mcp, uint8_t* data, size_t size);
bool mcp23s17_spi_read(MCP23S17* mcp, uint8_t* tx_data, size_t tx_size, uint8_t* rx_data, size_t rx_size);
bool mcp23s17_init(MCP23S17* mcp, uint8_t address, FuriHalSpiBusHandle* spi, const GpioPin* cs_pin);
bool mcp23s17_write_reg(MCP23S17* mcp, uint8_t reg, uint8_t value);
bool mcp23s17_read_reg(MCP23S17* mcp, uint8_t reg, uint8_t* value);
bool mcp23s17_port_mode(MCP23S17* mcp, MCP23S17Port port, uint8_t mode);
bool mcp23s17_digital_write(MCP23S17* mcp, uint8_t pin, MCP23S17Port port, bool value);
bool mcp23s17_digital_read(MCP23S17* mcp, uint8_t pin, MCP23S17Port port, bool* value);
bool mcp23s17_write_port(MCP23S17* mcp, MCP23S17Port port, uint8_t value);
bool mcp23s17_read_port(MCP23S17* mcp, MCP23S17Port port, uint8_t* value);
bool mcp23s17_is_connected(MCP23S17* mcp);
void mcp23s17_deinit(MCP23S17* mcp);

#endif // MCP23S17_API_H 
