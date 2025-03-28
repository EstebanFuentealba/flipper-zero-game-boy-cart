#include "mcp23s17_api.h"
#include <string.h>

// SPI write con CS control
bool mcp23s17_spi_write(MCP23S17* mcp, uint8_t* data, size_t size) {
    furi_hal_spi_acquire(mcp->spi);
    furi_hal_gpio_write(mcp->cs_pin, false);
    bool result = furi_hal_spi_bus_tx(mcp->spi, data, size, 100);
    furi_hal_gpio_write(mcp->cs_pin, true);
    furi_hal_spi_release(mcp->spi);
    return result;
}

// SPI read con CS control
bool mcp23s17_spi_read(MCP23S17* mcp, uint8_t* tx_data, size_t tx_size, uint8_t* rx_data, size_t rx_size) {
    furi_hal_spi_acquire(mcp->spi);
    furi_hal_gpio_write(mcp->cs_pin, false);
    
    // Transmitir datos
    bool tx_result = furi_hal_spi_bus_tx(mcp->spi, tx_data, tx_size, 100);
    
    // Recibir datos
    bool rx_result = false;
    if(tx_result) {
        rx_result = furi_hal_spi_bus_rx(mcp->spi, rx_data, rx_size, 100);
    }
    
    furi_hal_gpio_write(mcp->cs_pin, true);
    furi_hal_spi_release(mcp->spi);
    
    return tx_result && rx_result;
}

// Inicializa el MCP23S17
bool mcp23s17_init(MCP23S17* mcp, uint8_t address, FuriHalSpiBusHandle* spi, const GpioPin* cs_pin) {
    if(!mcp || !spi || !cs_pin) return false;
    
    // Inicializa la estructura
    mcp->address = address;
    mcp->spi = spi;
    mcp->cs_pin = cs_pin;
    mcp->initialized = false;
    memset(mcp->reg_cache, 0, sizeof(mcp->reg_cache));
    
    // Configurar CS pin como output
    furi_hal_gpio_init_simple(mcp->cs_pin, GpioModeOutputPushPull);
    furi_hal_gpio_write(mcp->cs_pin, true);
    
    // Escribir al registro IOCON para configurar el dispositivo
    uint8_t iocon_setup[3] = {
        MCP23S17_WRITE_OPCODE | (mcp->address << 1),
        MCP23S17_IOCONA,
        IOCON_HAEN
    };
    if (!mcp23s17_spi_write(mcp, iocon_setup, sizeof(iocon_setup))) {
        FURI_LOG_E("MCP23S17", "Failed to write IOCON");
        return false;
    }
    
    // Configurar todos los pines como salidas por defecto
    uint8_t iodir_setup[4] = {
        MCP23S17_WRITE_OPCODE | (mcp->address << 1),
        MCP23S17_IODIRA,
        0x00, // Port A como salidas
        0x00  // Port B como salidas
    };
    if (!mcp23s17_spi_write(mcp, iodir_setup, sizeof(iodir_setup))) {
        FURI_LOG_E("MCP23S17", "Failed to configure IODIR");
        return false;
    }
    
    // Configurar todos los pines a nivel bajo
    uint8_t gpio_setup[4] = {
        MCP23S17_WRITE_OPCODE | (mcp->address << 1),
        MCP23S17_OLATA,
        0x00, // Port A bajo
        0x00  // Port B bajo
    };
    if (!mcp23s17_spi_write(mcp, gpio_setup, sizeof(gpio_setup))) {
        FURI_LOG_E("MCP23S17", "Failed to configure GPIO");
        return false;
    }
    
    // Verificar comunicación leyendo IODIR
    uint8_t tx_buf[2] = {
        MCP23S17_READ_OPCODE | (mcp->address << 1),
        MCP23S17_IODIRA
    };
    uint8_t rx_buf[2] = {0};
    
    if (!mcp23s17_spi_read(mcp, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf))) {
        FURI_LOG_E("MCP23S17", "Failed to read IODIR for verification");
        return false;
    }
    
    FURI_LOG_I("MCP23S17", "IODIRA: 0x%02X, IODIRB: 0x%02X", rx_buf[0], rx_buf[1]);
    
    if (rx_buf[0] != 0x00 || rx_buf[1] != 0x00) {
        FURI_LOG_E("MCP23S17", "IODIR verification failed");
        return false;
    }
    
    mcp->initialized = true;
    FURI_LOG_I("MCP23S17", "Initialization successful");
    return true;
}

// Escribe a un registro del MCP23S17
bool mcp23s17_write_reg(MCP23S17* mcp, uint8_t reg, uint8_t value) {
    if(!mcp || !mcp->initialized) return false;
    
    uint8_t buffer[3] = {
        MCP23S17_WRITE_OPCODE | (mcp->address << 1),
        reg,
        value
    };
    
    bool result = mcp23s17_spi_write(mcp, buffer, sizeof(buffer));
    if(result && reg < sizeof(mcp->reg_cache)) {
        mcp->reg_cache[reg] = value;
    }
    
    return result;
}

// Lee un registro del MCP23S17
bool mcp23s17_read_reg(MCP23S17* mcp, uint8_t reg, uint8_t* value) {
    if(!mcp || !mcp->initialized || !value) return false;
    
    uint8_t tx_buf[2] = {
        MCP23S17_READ_OPCODE | (mcp->address << 1),
        reg
    };
    uint8_t rx_buf[1] = {0};
    
    bool result = mcp23s17_spi_read(mcp, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf));
    if(result) {
        *value = rx_buf[0];
    }
    
    return result;
}

// Configura todos los pines de un puerto
bool mcp23s17_port_mode(MCP23S17* mcp, MCP23S17Port port, uint8_t mode) {
    if(!mcp || !mcp->initialized) return false;
    
    uint8_t iodir_reg = (port == MCP23S17_PORT_A) ? MCP23S17_IODIRA : MCP23S17_IODIRB;
    uint8_t gppu_reg = (port == MCP23S17_PORT_A) ? MCP23S17_GPPUA : MCP23S17_GPPUB;
    
    // Modo: 0 = todos salidas, 1 = todos entradas, 2 = todos entradas con pull-up
    uint8_t iodir_val = (mode == 0) ? 0x00 : 0xFF;
    uint8_t gppu_val = (mode == 2) ? 0xFF : 0x00;
    
    if(!mcp23s17_write_reg(mcp, iodir_reg, iodir_val)) return false;
    if(!mcp23s17_write_reg(mcp, gppu_reg, gppu_val)) return false;
    
    return true;
}

// Escribe a un pin individual
bool mcp23s17_digital_write(MCP23S17* mcp, uint8_t pin, MCP23S17Port port, bool value) {
    if(!mcp || !mcp->initialized || pin > 7) return false;
    
    uint8_t pin_bit = 1 << pin;
    uint8_t olat_reg = (port == MCP23S17_PORT_A) ? MCP23S17_OLATA : MCP23S17_OLATB;
    
    // Leer valor actual
    uint8_t olat_val;
    if(!mcp23s17_read_reg(mcp, olat_reg, &olat_val)) return false;
    
    // Modificar bit específico
    if(value) {
        olat_val |= pin_bit;    // Establecer a 1
    } else {
        olat_val &= ~pin_bit;   // Establecer a 0
    }
    
    // Escribir valor actualizado
    if(!mcp23s17_write_reg(mcp, olat_reg, olat_val)) return false;
    
    // Verificar
    uint8_t gpio_reg = (port == MCP23S17_PORT_A) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    uint8_t gpio_val;
    if(!mcp23s17_read_reg(mcp, gpio_reg, &gpio_val)) {
        FURI_LOG_W("MCP23S17", "Failed to verify pin write");
    } else {
        bool pin_state = (gpio_val & pin_bit) != 0;
        if(pin_state != value) {
            FURI_LOG_W("MCP23S17", "Pin verification failed. Expected: %d, Got: %d", 
                      value, pin_state);
        }
    }
    
    return true;
}

// Escribe a un puerto completo
bool mcp23s17_write_port(MCP23S17* mcp, MCP23S17Port port, uint8_t value) {
    if(!mcp || !mcp->initialized) return false;
    
    uint8_t olat_reg = (port == MCP23S17_PORT_A) ? MCP23S17_OLATA : MCP23S17_OLATB;
    
    if(!mcp23s17_write_reg(mcp, olat_reg, value)) return false;
    
    // Verificar
    uint8_t gpio_reg = (port == MCP23S17_PORT_A) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    uint8_t gpio_val;
    if(mcp23s17_read_reg(mcp, gpio_reg, &gpio_val)) {
        // FURI_LOG_D("MCP23S17", "Port %c value: 0x%02X (expected: 0x%02X)", 
        //            (port == MCP23S17_PORT_A) ? 'A' : 'B', gpio_val, value);
    }
    
    return true;
}

// Lee un pin individual
bool mcp23s17_digital_read(MCP23S17* mcp, uint8_t pin, MCP23S17Port port, bool* value) {
    if(!mcp || !mcp->initialized || pin > 7 || !value) return false;
    
    uint8_t pin_bit = 1 << pin;
    uint8_t gpio_reg = (port == MCP23S17_PORT_A) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    
    uint8_t gpio_val;
    if(!mcp23s17_read_reg(mcp, gpio_reg, &gpio_val)) return false;
    
    *value = (gpio_val & pin_bit) != 0;
    return true;
}

// Lee un puerto completo
bool mcp23s17_read_port(MCP23S17* mcp, MCP23S17Port port, uint8_t* value) {
    if(!mcp || !mcp->initialized || !value) return false;
    
    uint8_t gpio_reg = (port == MCP23S17_PORT_A) ? MCP23S17_GPIOA : MCP23S17_GPIOB;
    
    return mcp23s17_read_reg(mcp, gpio_reg, value);
}

// Función para verificar la presencia del dispositivo
bool mcp23s17_is_connected(MCP23S17* mcp) {
    if(!mcp) return false;
    
    uint8_t test_val;
    bool result = mcp23s17_read_reg(mcp, MCP23S17_IODIRA, &test_val);
    
    return result;
}

// Función para desinicializar el MCP23S17
void mcp23s17_deinit(MCP23S17* mcp) {
    if (!mcp) return;
    
    // Liberar el pin CS
    if (mcp->cs_pin) {
        furi_hal_gpio_init_simple(mcp->cs_pin, GpioModeAnalog);
    }
    
    // Limpiar la estructura
    mcp->initialized = false;
    mcp->address = 0;
    mcp->spi = NULL;
    mcp->cs_pin = NULL;
    memset(mcp->reg_cache, 0, sizeof(mcp->reg_cache));
} 