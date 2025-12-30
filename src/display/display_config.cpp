#include "display_config.h"

// LCD configuration for TENSTAR ROBOT 2.4" TFT ST7789V - matching working test
const struct st7789_config lcd_config = {
    .spi      = DISPLAY_SPI_PORT,      // SPI1
    .gpio_din = DISPLAY_SPI_MOSI,      // MOSI (Pin 11)
    .gpio_clk = DISPLAY_SPI_SCK,       // SCK (Pin 10)
    .gpio_cs  = DISPLAY_CS_PIN,        // CS (Pin 9)
    .gpio_dc  = DISPLAY_DC_PIN,        // DC (Pin 8)
    .gpio_rst = DISPLAY_RESET_PIN,     // RST (Pin 12)
    .gpio_bl  = DISPLAY_BACKLIGHT_PIN, // BL (Pin 13)
};

// Dimensions matching working test
const int lcd_width = DISPLAY_WIDTH;
const int lcd_height = DISPLAY_HEIGHT;

// Initialize display function
void init_display() {
    st7789_init(&lcd_config, lcd_width, lcd_height);
}