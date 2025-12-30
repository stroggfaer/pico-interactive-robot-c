#ifndef EMOTIONS_H
#define EMOTIONS_H

#include "states.h"
#include "mrx.h"
#include <string>

// Display is now handled directly via C driver in emotions.cpp
// No global display pointer needed

// Display and matrix constants
#define PIXEL_SIZE 20
#define MATRIX_WIDTH (12 * PIXEL_SIZE)
#define MATRIX_HEIGHT (12 * PIXEL_SIZE)
#define DISPLAY_WIDTH 240
#define DISPLAY_HEIGHT 320
#define X_OFFSET ((DISPLAY_WIDTH - MATRIX_WIDTH) / 2)
#define Y_OFFSET ((DISPLAY_HEIGHT - MATRIX_HEIGHT) / 2)

// Vowels for syllable counting
#define VOWELS "аеёиоуыэюяaeiouy"

// Function declarations for emotion handling
void reset_matrix();
void draw_matrix(const uint8_t matrix[MATRIX_ROWS][MATRIX_COLS], int pixel_size = PIXEL_SIZE, bool force_redraw = false);
int count_syllables(const std::string& text);

// Animation functions
void neutral(double speed, NeutralState& state);
void smile_pixel(double speed, AnimState& state, uint32_t duration);
void smile_love_pixel(double speed, AnimState& state, uint32_t duration);
void embarrassed_pixel(double speed, AnimState& state);
void scary_pixel(double speed, AnimState& state, uint32_t duration);
void happy_pixel(double speed, AnimState& state, uint32_t duration);
void sad_pixel(double speed, AnimState& state, uint32_t duration);
void surprise_pixel(double speed, AnimState& state, uint32_t duration);

// Talking functions
void talking_pixel(uint32_t duration, double speed, TalkingState& state,
                  const std::string& text, double mouth_speed, const std::string& emotion);

// Animation logic functions
void anime_logic(AnimState& state, double speed, uint32_t duration,
                const uint8_t* matrix_start, const uint8_t* matrix_anim_a,
                const uint8_t* matrix_anim_b, const uint8_t* matrix_anim_c,
                const uint8_t* matrix_end);

void talking_logic(TalkingState& state, const std::string& text, uint32_t duration,
                  double speed, double mouth_speed,
                  const uint8_t open_matrix[MATRIX_ROWS][MATRIX_COLS],
                  const uint8_t closed_matrix[MATRIX_ROWS][MATRIX_COLS],
                  const uint8_t neutral_matrix[MATRIX_ROWS][MATRIX_COLS]);

#endif // EMOTIONS_H