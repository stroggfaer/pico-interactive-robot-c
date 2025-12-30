#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include "hardware/spi.h"

extern "C" {
#include "pico/st7789.h"
}

// Pin definitions for TENSTAR ROBOT 2.4" TFT ST7789V 320x240
#define DISPLAY_SPI_PORT spi1
#define DISPLAY_SPI_SCK 10
#define DISPLAY_SPI_MOSI 11
#define DISPLAY_DC_PIN 8
#define DISPLAY_CS_PIN 9
#define DISPLAY_RESET_PIN 12
#define DISPLAY_BACKLIGHT_PIN 13

// Display dimensions
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320

// LCD configuration for TENSTAR ROBOT 2.4" TFT ST7789V
extern const struct st7789_config lcd_config;
extern const int lcd_width;
extern const int lcd_height;

// Initialize display
void init_display();

#endif // DISPLAY_CONFIG_H