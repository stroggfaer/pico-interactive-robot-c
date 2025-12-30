#ifndef COLORS_H
#define COLORS_H

#include <cstdint>

// Color conversion function (RGB565)
inline uint16_t color565(uint8_t red, uint8_t green, uint8_t blue) {
    return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

// Basic colors
const uint16_t WHITE = color565(255, 255, 255);
const uint16_t BLACK = color565(0, 0, 0);
const uint16_t RED = color565(255, 0, 0);
const uint16_t GREEN = color565(0, 255, 0);
const uint16_t BLUE = color565(0, 0, 255);
const uint16_t YELLOW = color565(255, 255, 0);
const uint16_t CYAN = color565(0, 255, 255);
const uint16_t MAGENTA = color565(255, 0, 255);

// Additional colors
const uint16_t ORANGE = color565(255, 165, 0);
const uint16_t PURPLE = color565(128, 0, 128);
const uint16_t GRAY = color565(128, 128, 128);
const uint16_t LIGHT_GRAY = color565(192, 192, 192);
const uint16_t DARK_GRAY = color565(64, 64, 64);

// Color gradients
const uint16_t LIGHT_BLUE = color565(173, 216, 230);
const uint16_t PINK = color565(255, 192, 203);
const uint16_t BROWN = color565(165, 42, 42);
const uint16_t GOLD = color565(255, 215, 0);
const uint16_t SILVER = color565(192, 192, 192);

// Style colors
const uint16_t STYLE_FACE = WHITE;
const uint16_t STYLE_FACE_RED = RED;
const uint16_t STYLE_EYE = BLACK;
const uint16_t STYLE_MOUTH = WHITE;
const uint16_t STYLE_BG = BLACK;

#endif // COLORS_H