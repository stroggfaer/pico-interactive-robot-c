#include "states.h"
#include <cstdlib>
#include <ctime>
#include <string>

// Get neutral state
NeutralState get_neutral_state() {
    NeutralState state;
    state.pupil_direction = (rand() % 7) - 3; // -3 to 3
    return state;
}

// Get talking state
TalkingState get_talking_state() {
    TalkingState state;
    return state;
}

// Get animation state
AnimState get_anim_state() {
    AnimState state;
    return state;
}

// Reset neutral state
NeutralState reset_neutral_state() {
    return get_neutral_state();
}

// Reset talking state
TalkingState reset_talking_state() {
    return get_talking_state();
}

// Reset animation state
AnimState reset_anim_state() {
    return get_anim_state();
}

// Generic reset function
void* reset_state(const std::string& emotion) {
    if (emotion == "neutral") {
        return new NeutralState(get_neutral_state());
    } else if (emotion == "talking") {
        return new TalkingState(get_talking_state());
    } else {
        return new AnimState(get_anim_state());
    }
}