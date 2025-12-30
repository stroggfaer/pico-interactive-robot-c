#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <ctime>
#include "pico/stdlib.h"
#include "display_config.h"
#include "emotions.h"
#include "states.h"
#include "colors.h"

// Global display initialization flag
bool display_initialized = false;

// Global variables similar to Python
std::string current_emotion = "neutral";
std::string talking_emotion = "";
double current_duration = 65.5;
double current_intensity = 0.4;
std::string current_text = "";
double current_mouth_speed = 0.5;
double emotion_timer = 0.0;
double anim_duration = 5.0;
double last_emotion_time = 0.0;

// Emotion states
std::map<std::string, void*> emotion_states;

// Function to get current time in seconds
double get_time() {
    return (double)to_ms_since_boot(get_absolute_time()) / 1000.0;
}

// Reset emotion state
void reset_emotion_state(const std::string& emotion) {
    emotion_states[emotion] = reset_state(emotion);
    emotion_timer = get_time();
    printf("[DEBUG] Reset state for %s\n", emotion.c_str());
}

// Emotion functions map
std::map<std::string, std::function<void(double)>> emotions;

// Initialize emotions map
void init_emotions() {
    emotions["neutral"] = [](double i) {
        NeutralState* state = static_cast<NeutralState*>(emotion_states["neutral"]);
        neutral(0.2 * i, *state);
    };

    emotions["smile"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["smile"]);
        smile_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["smile_love"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["smile_love"]);
        smile_love_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["embarrassed"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["embarrassed"]);
        embarrassed_pixel(current_mouth_speed * i, *state);
    };

    emotions["scary"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["scary"]);
        scary_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["happy"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["happy"]);
        happy_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["sad"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["sad"]);
        sad_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["surprise"] = [](double i) {
        AnimState* state = static_cast<AnimState*>(emotion_states["surprise"]);
        surprise_pixel(current_mouth_speed * i, *state, anim_duration);
    };

    emotions["talking"] = [](double i) {
        TalkingState* state = static_cast<TalkingState*>(emotion_states["talking"]);
        std::string emotion = talking_emotion.empty() ? "neutral" : talking_emotion;
        // Передаем правильные параметры: duration в секундах, не в мс
        talking_pixel((uint32_t)current_duration, current_intensity * i, *state,
                     current_text, current_mouth_speed, emotion);
    };
}

// Enhanced JSON parser for commands with debug output
std::map<std::string, std::string> parse_json(const std::string& json_str) {
    std::map<std::string, std::string> result;
    std::string key, value;
    bool in_key = false, in_value = false;
    bool in_string = false;
    char quote_char = 0;
    
    printf("[JSON_PARSE] Starting parse of: %s\n", json_str.c_str());
    
    for (size_t i = 0; i < json_str.length(); ++i) {
        char c = json_str[i];
        
        if (!in_string && (c == '{' || c == '}' || c == ',' || c == ':')) {
            if (!key.empty() && !value.empty()) {
                printf("[JSON_PARSE] Found pair: '%s' = '%s'\n", key.c_str(), value.c_str());
                result[key] = value;
                key.clear();
                value.clear();
            }
            if (c == ':') {
                in_value = true;
                in_key = false;
                printf("[JSON_PARSE] Switching to value mode, key='%s'\n", key.c_str());
            } else if (c == ',') {
                in_key = true;
                in_value = false;
                printf("[JSON_PARSE] Switching to key mode\n");
            } else if (c == '{') {
                in_key = true;
                printf("[JSON_PARSE] JSON start, entering key mode\n");
            }
            continue;
        }

        if ((c == '"' || c == '\'') && (!in_string || quote_char == c)) {
            in_string = !in_string;
            quote_char = in_string ? c : 0;
            printf("[JSON_PARSE] String mode: %s, quote_char: %c\n", in_string ? "ON" : "OFF", quote_char);
            continue;
        }

        if (in_string) {
            if (in_key) {
                key += c;
            } else if (in_value) {
                value += c;
            }
        } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            if (in_key) {
                key += c;
            } else if (in_value) {
                value += c;
            }
        }
    }

    if (!key.empty() && !value.empty()) {
        printf("[JSON_PARSE] Final pair: '%s' = '%s'\n", key.c_str(), value.c_str());
        result[key] = value;
    }
    
    printf("[JSON_PARSE] Parse complete, found %d pairs\n", result.size());
    return result;
}

// Read command from stdin (non-blocking with improved JSON detection)
std::string read_command() {
    static std::string buffer = "";
    static bool in_json = false;
    static int brace_count = 0;
    
    // Read all available characters
    while (true) {
        int result = getchar_timeout_us(1000); // Small timeout to gather chars
        if (result == PICO_ERROR_TIMEOUT) {
            break;
        }
        
        char c = (char)result;
        printf("[DEBUG] Char received: %c (0x%02x), buffer_len=%d\n", c, c, buffer.length());
        
        // Skip carriage return
        if (c == '\r') {
            continue;
        }
        
        // Handle newline
        if (c == '\n') {
            if (in_json && brace_count == 0 && !buffer.empty()) {
                printf("[DEBUG] Complete JSON command detected: %s\n", buffer.c_str());
                std::string cmd = buffer;
                buffer.clear();
                in_json = false;
                brace_count = 0;
                return cmd;
            } else if (!buffer.empty()) {
                printf("[DEBUG] Non-JSON or incomplete command: %s\n", buffer.c_str());
                buffer.clear();
                in_json = false;
                brace_count = 0;
            }
            continue;
        }
        
        // Add character to buffer
        buffer += c;
        
        // JSON detection logic
        if (c == '{') {
            if (!in_json) {
                in_json = true;
                brace_count = 1;
                printf("[DEBUG] JSON start detected\n");
            } else {
                brace_count++;
            }
        } else if (c == '}' && in_json) {
            brace_count--;
            if (brace_count == 0) {
                printf("[DEBUG] JSON end detected, buffer: %s\n", buffer.c_str());
                // Don't return yet, wait for newline or check if complete
            }
        }
        
        // Safety check - if buffer gets too large, reset
        if (buffer.length() > 2048) {
            printf("[ERROR] Buffer overflow, resetting\n");
            buffer.clear();
            in_json = false;
            brace_count = 0;
        }
    }
    
    // If we have a complete JSON (braces balanced) and no more chars, return it
    if (in_json && brace_count == 0 && !buffer.empty()) {
        printf("[DEBUG] Complete JSON ready: %s\n", buffer.c_str());
        std::string cmd = buffer;
        buffer.clear();
        in_json = false;
        brace_count = 0;
        return cmd;
    }
    
    return "";
}

// Main function
int main() {
    stdio_init_all();
    printf("[INFO] Starting Interactive Robot (C++ version)...\n");

    // Initialize display using structured config
    init_display();
    display_initialized = true;
    printf("[INFO] TFT initialized successfully\n");

    // Initialize emotion states
    std::vector<std::string> emotion_names = {
        "neutral", "smile", "smile_love", "embarrassed",
        "scary", "happy", "sad", "surprise", "talking"
    };

    for (const auto& name : emotion_names) {
        emotion_states[name] = reset_state(name.c_str());
    }

    // Initialize emotions map
    init_emotions();

    // Set initial emotion
    reset_emotion_state(current_emotion);
    if (display_initialized) {
        emotions[current_emotion](current_intensity);
    }

    printf("Pico started, waiting for JSON commands...\n");

    bool new_command_received = false;
    last_emotion_time = get_time();

    while (true) {
            std::string command_json = read_command();
            if (!command_json.empty()) {
                printf("[JSON] Raw input: '%s'\n", command_json.c_str());
                printf("[JSON] Input length: %d\n", command_json.length());

                auto command = parse_json(command_json);
                printf("[JSON] Parsed %d fields\n", command.size());

                if (command.find("emotion") == command.end()) {
                    printf("[ERROR] Invalid command structure - missing 'emotion' field\n");
                    printf("[ERROR] Available fields: ");
                    for (const auto& pair : command) {
                        printf("'%s'='%s' ", pair.first.c_str(), pair.second.c_str());
                    }
                    printf("\n");
                    continue;
                }

                std::string new_emotion = command["emotion"];
                printf("[PARSE] emotion=%s\n", new_emotion.c_str());
                
                if (command.find("duration") != command.end()) {
                    current_duration = std::stod(command["duration"]);
                    printf("[PARSE] duration=%.2f\n", current_duration);
                }
                if (command.find("intensity") != command.end()) {
                    current_intensity = std::stod(command["intensity"]);
                    printf("[PARSE] intensity=%.2f\n", current_intensity);
                }
                if (command.find("mouth_speed") != command.end()) {
                    current_mouth_speed = std::stod(command["mouth_speed"]);
                    printf("[PARSE] mouth_speed=%.2f\n", current_mouth_speed);
                }
                if (command.find("text") != command.end()) {
                    current_text = command["text"];
                    printf("[PARSE] text='%s'\n", current_text.c_str());
                }
                if (command.find("anim_duration") != command.end()) {
                    anim_duration = std::stod(command["anim_duration"]);
                    printf("[PARSE] anim_duration=%.2f\n", anim_duration);
                }
                if (command.find("talking_emotion") != command.end()) {
                    talking_emotion = command["talking_emotion"];
                    printf("[PARSE] talking_emotion=%s\n", talking_emotion.c_str());
                }

                printf("[COMMAND] Complete parsed command: emotion=%s, talking_emotion=%s, duration=%.2f, intensity=%.2f, text='%s', mouth_speed=%.2f\n",
                       new_emotion.c_str(), talking_emotion.c_str(), current_duration,
                       current_intensity, current_text.c_str(), current_mouth_speed);

                if (emotions.find(new_emotion) == emotions.end()) {
                    printf("[ERROR] Emotion '%s' not defined, using 'neutral'\n", new_emotion.c_str());
                    new_emotion = "neutral";
                }

                current_emotion = new_emotion;
                new_command_received = true;
                printf("[SUCCESS] Command processed successfully\n");
            }

            if (new_command_received && get_time() - last_emotion_time > 0.5) {
                if (display_initialized) {
                    printf("[EMOTION] Switching to emotion: %s\n", current_emotion.c_str());
                    reset_emotion_state(current_emotion);
                    emotions[current_emotion](current_intensity);
                    printf("[EMOTION] Successfully switched to %s\n", current_emotion.c_str());
                }
                last_emotion_time = get_time();
                new_command_received = false;
                
                // Send confirmation response
                printf("{\"status\": \"ok\", \"emotion\": \"%s\", \"timestamp\": %.2f}\n", 
                       current_emotion.c_str(), get_time());
            }

            if (display_initialized) {
                emotions[current_emotion](current_intensity);

                TalkingState* talking_state = static_cast<TalkingState*>(emotion_states["talking"]);
                if (!talking_state->talking && !current_text.empty()) {
                    current_text.clear();
                    talking_emotion.clear();
                }
            }

            if (get_time() - emotion_timer >= current_duration) {
                if (current_emotion != "neutral") {
                    std::string finished_emotion = current_emotion;
                    printf("[TIMEOUT] Emotion %s duration expired (%.2f >= %.2f)\n", 
                           finished_emotion.c_str(), get_time() - emotion_timer, current_duration);
                    current_emotion = "neutral";
                    current_text.clear();
                    talking_emotion.clear();
                    if (display_initialized) {
                        reset_emotion_state(current_emotion);
                        emotions[current_emotion](current_intensity);
                        printf("[TIMEOUT] Auto switched to neutral\n");
                    }
                    // Output finished event
                    printf("{\"event\": \"emotion_finished\", \"emotion\": \"%s\"}\n", finished_emotion.c_str());
                }
            }

            sleep_ms(1); // Минимальная пауза для предотвращения перегрузки процессора
    }

    return 0;
}