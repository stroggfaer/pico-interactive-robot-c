#ifndef STATES_H
#define STATES_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <functional>

// Neutral state structure
struct NeutralState {
    uint32_t last_blink = 0;
    uint32_t start_time = 0;
    uint32_t sleep_start = 0;
    uint32_t last_yawn = 0;
    int8_t pupil_direction = 0;
    bool blink = false;
    uint8_t blink_phase = 0;
    uint32_t blink_start = 0;
    bool matrix = true;
    uint8_t sleep_blink_phase = 0;
};

// Talking state structure
struct TalkingState {
    bool talking = false;
    uint8_t frame = 0;
    uint32_t start_time = 0;
    uint32_t last_frame = 0;
    std::string emotion = "neutral";
    uint8_t syllables = 1;
    uint8_t program_step = 0;
    uint32_t last_step_time = 0;
};

// Animation state structure
struct AnimState {
    bool animating = false;
    uint8_t frame = 0;
    uint32_t start_time = 0;
    uint32_t last_frame = 0;
    uint32_t cycle_count = 0;
};

// Function declarations
NeutralState get_neutral_state();
TalkingState get_talking_state();
AnimState get_anim_state();

// Reset state functions
NeutralState reset_neutral_state();
TalkingState reset_talking_state();
AnimState reset_anim_state();

// Generic reset function
void* reset_state(const std::string& emotion);

#endif // STATES_H