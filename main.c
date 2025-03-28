#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <notification/notification_messages.h>
#include <furi_hal_power.h>
#include <furi_hal_spi.h>
// Incluir la API que creamos
// En una aplicación real, esto sería un archivo separado
#include "mcp23s17_api.h"
#include "gb_cart.h"

// Dirección I2C del MCP23S17 (0x20 por defecto)
#define MCP23S17_ADDRESS 0x20

typedef struct {
    FuriMutex* mutex;
    MCP23S17* mcp1;
    MCP23S17* mcp2;
    GBCartInfo cart_info;
    bool cart_detected;
    bool reading;
    int scroll_position;  // Nueva variable para el scroll
} GBCartApp;

static void render_callback(Canvas* canvas, void* ctx) {
    GBCartApp* app = ctx;
    furi_mutex_acquire(app->mutex, FuriWaitForever);

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Game Boy Cart Reader");

    if (app->reading) {
        canvas_draw_str(canvas, 0, 30, "Leyendo cartucho...");
    } else if (app->cart_detected) {
        // Mostrar información del cartucho con scroll
        char buffer[32];
        int y_pos = 30 - app->scroll_position;  // Posición Y inicial ajustada por scroll
        
        // Título
        canvas_draw_str(canvas, 0, y_pos, "Titulo:");
        canvas_draw_str(canvas, 0, y_pos + 10, app->cart_info.title);
        
        // Tipo de cartucho
        char type_str[32];
        gb_cart_get_type_string(type_str, app->cart_info.cart_type);
        canvas_draw_str(canvas, 0, y_pos + 20, "Tipo:");
        canvas_draw_str(canvas, 0, y_pos + 30, type_str);
        
        // Información adicional
        snprintf(buffer, sizeof(buffer), "ROM: %luKB (%d banks)", 
                app->cart_info.rom_size / 1024, app->cart_info.rom_banks);
        canvas_draw_str(canvas, 0, y_pos + 40, buffer);
        
        snprintf(buffer, sizeof(buffer), "RAM: %luKB (%d banks)", 
                app->cart_info.ram_size / 1024, app->cart_info.ram_banks);
        canvas_draw_str(canvas, 0, y_pos + 50, buffer);
        
        snprintf(buffer, sizeof(buffer), "Bateria: %s", 
                app->cart_info.has_battery ? "SI" : "NO");
        canvas_draw_str(canvas, 0, y_pos + 60, buffer);
        
        snprintf(buffer, sizeof(buffer), "SGB: %s", 
                app->cart_info.has_sgb ? "SI" : "NO");
        canvas_draw_str(canvas, 0, y_pos + 70, buffer);
        
        snprintf(buffer, sizeof(buffer), "Checksum: 0x%02X", 
                app->cart_info.checksum);
        canvas_draw_str(canvas, 0, y_pos + 80, buffer);

        // Dibujar indicador de scroll
        canvas_set_font(canvas, FontSecondary);
        canvas_draw_str(canvas, 0, 120, "Arriba/Abajo: Scroll");
    } else {
        canvas_draw_str(canvas, 0, 30, "No hay cartucho detectado");
        canvas_draw_str(canvas, 0, 40, "Presiona OK para leer");
    }

    furi_mutex_release(app->mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t gb_cart_app(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    
    GBCartApp* app = malloc(sizeof(GBCartApp));
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->cart_detected = false;
    app->reading = false;
    app->scroll_position = 0;  // Inicializar posición de scroll
    
    // Configurar la interfaz gráfica
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, app);
    view_port_input_callback_set(view_port, input_callback, event_queue);
    
    // Registrar el viewport con GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    NotificationApp* notifications = furi_record_open(RECORD_NOTIFICATION);
    
    // Configurar SPI
    FuriHalSpiBusHandle* spi = &furi_hal_spi_bus_handle_external;
    const GpioPin* cs_pin1 = &gpio_ext_pa4;
    const GpioPin* cs_pin2 = &gpio_ext_pc3;
    
    // Inicializar MCP23S17
    app->mcp1 = malloc(sizeof(MCP23S17));
    app->mcp2 = malloc(sizeof(MCP23S17));
    
    if (!mcp23s17_init(app->mcp1, 0, spi, cs_pin1) || 
        !mcp23s17_init(app->mcp2, 1, spi, cs_pin2)) {
        FURI_LOG_E("MCP23S17", "Inicialización fallida");
        notification_message(notifications, &sequence_error);
    } else {
        FURI_LOG_I("MCP23S17", "Inicialización exitosa");
        notification_message(notifications, &sequence_success);
        
        // Inicializar el lector de cartuchos
        if (!gb_cart_init(app->mcp1, app->mcp2)) {
            FURI_LOG_E("GB_CART", "Inicialización fallida");
            notification_message(notifications, &sequence_error);
        }
    }
    
    // Bucle principal
    InputEvent event;
    bool running = true;
    while (running) {
        if (furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(app->mutex, FuriWaitForever);
            
            if (event.type == InputTypeShort) {
                switch(event.key) {
                    case InputKeyBack:
                        running = false;
                        break;
                    case InputKeyOk:
                        if (!app->reading) {
                            app->reading = true;
                            app->cart_detected = gb_cart_read_info(&app->cart_info);
                            app->reading = false;
                            
                            if (app->cart_detected) {
                                notification_message(notifications, &sequence_success);
                            } else {
                                notification_message(notifications, &sequence_error);
                            }
                        }
                        break;
                    case InputKeyUp:
                        if (app->cart_detected && app->scroll_position > 0) {
                            app->scroll_position -= 10;
                        }
                        break;
                    case InputKeyDown:
                        if (app->cart_detected && app->scroll_position < 50) {
                            app->scroll_position += 10;
                        }
                        break;
                    case InputKeyLeft:
                    case InputKeyRight:
                    case InputKeyMAX:
                        // Ignorar estas teclas
                        break;
                }
            }
            
            furi_mutex_release(app->mutex);
        }
        
        view_port_update(view_port);
    }
    
    // Limpieza
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_mutex_free(app->mutex);
    
    if (app->mcp1) {
        mcp23s17_deinit(app->mcp1);
        free(app->mcp1);
    }
    if (app->mcp2) {
        mcp23s17_deinit(app->mcp2);
        free(app->mcp2);
    }
    
    free(app);
    
    return 0;
}