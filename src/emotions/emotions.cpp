#include "emotions.h"
#include "colors.h"
#include <cstring>
#include <cstdlib>
#include "pico/stdlib.h"
#include <algorithm>
#include <vector>

// Use C driver directly
extern "C" {
#include "pico/st7789.h"
}

// Animation system constants
static const uint32_t ANIMATION_FPS = 60;  // 60 FPS для плавности
static const uint32_t FRAME_TIME_US = 1000000 / ANIMATION_FPS;  // 16.67ms в микросекундах
static const uint32_t MIN_FRAME_DURATION_MS = 50;   // Минимальная длительность кадра
static const uint32_t MAX_FRAME_DURATION_MS = 500;  // Максимальная длительность кадра

// Global variables
static uint8_t prev_matrix[MATRIX_ROWS][MATRIX_COLS];
static uint8_t current_matrix[MATRIX_ROWS][MATRIX_COLS];
static bool matrix_initialized = false;
static uint32_t last_draw_time = 0;
static bool animation_dirty = false;

// Animation interpolation system
struct AnimationFrame {
    const uint8_t (*matrix)[MATRIX_COLS];
    uint32_t duration_ms;
    const char* name;
};

static std::vector<AnimationFrame> current_animation_sequence;
static size_t current_frame_index = 0;
static uint32_t frame_start_time = 0;

void reset_matrix() {
    matrix_initialized = false;
    animation_dirty = true;
    current_animation_sequence.clear();
    current_frame_index = 0;
    memset(current_matrix, 0, sizeof(current_matrix));
    printf("[ANIM_SYS] Matrix reset\n");
}

// Продвинутая функция отрисовки с минимальным мерцанием
void st7789_fill_rect_optimized(int x, int y, int width, int height, uint16_t color) {
    // Оптимизированная заливка прямоугольника
    for (int py = 0; py < height; py++) {
        st7789_set_cursor(x, y + py);
        for (int px = 0; px < width; px++) {
            st7789_put(color);
        }
    }
}

void draw_matrix(const uint8_t matrix[MATRIX_ROWS][MATRIX_COLS], int pixel_size, bool force_redraw) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Ограничиваем частоту обновления для плавности
    if (!force_redraw && (current_time - last_draw_time) < (1000 / ANIMATION_FPS)) {
        return; // Пропускаем слишком частые обновления
    }
    
    last_draw_time = current_time;
    
    if (!matrix_initialized || force_redraw) {
        // Полная перерисовка с оптимизацией
        st7789_fill(STYLE_BG);
        
        // Отрисовываем все видимые пиксели
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (matrix[row][col] == 1) {
                    int x = X_OFFSET + col * pixel_size;
                    int y = Y_OFFSET + row * pixel_size;
                    st7789_fill_rect_optimized(x, y, pixel_size, pixel_size, STYLE_FACE);
                }
            }
        }
        matrix_initialized = true;
    } else {
        // Инкрементальное обновление - только изменённые пиксели
        bool has_changes = false;
        
        for (int row = 0; row < MATRIX_ROWS; row++) {
            for (int col = 0; col < MATRIX_COLS; col++) {
                if (matrix[row][col] != prev_matrix[row][col]) {
                    has_changes = true;
                    int x = X_OFFSET + col * pixel_size;
                    int y = Y_OFFSET + row * pixel_size;
                    uint16_t color = (matrix[row][col] == 1) ? STYLE_FACE : STYLE_BG;
                    st7789_fill_rect_optimized(x, y, pixel_size, pixel_size, color);
                }
            }
        }
        
        if (has_changes) {
            printf("[ANIM_SYS] Incremental update completed\n");
        }
    }
    
    // Сохраняем текущее состояние
    memcpy(prev_matrix, matrix, sizeof(prev_matrix));
    memcpy(current_matrix, matrix, sizeof(current_matrix));
}

// Продвинутая система анимации для естественного разговора
void setup_talking_animation(const std::string& text, uint32_t total_duration_ms, double mouth_speed,
                            const uint8_t open_matrix[MATRIX_ROWS][MATRIX_COLS],
                            const uint8_t closed_matrix[MATRIX_ROWS][MATRIX_COLS]) {
    current_animation_sequence.clear();
    
    if (text.empty() || total_duration_ms == 0) {
        return;
    }
    
    int syllables = count_syllables(text);
    
    // Более реалистичные расчёты на основе скорости речи
    // Средняя скорость речи: 3-5 слогов в секунду
    // mouth_speed влияет на интенсивность движения рта
    
    // Базовая частота открытия рта (слогов в секунду)
    double syllables_per_second = 3.5; // Естественная скорость речи
    
    // mouth_speed влияет на активность рта: 0.1 = очень активно, 1.0 = спокойно
    double activity_factor = 1.0 / mouth_speed; // Инвертируем для логичности
    syllables_per_second *= activity_factor;
    
    // Рассчитываем время на один цикл открытие-закрытие
    uint32_t cycle_duration_ms = (uint32_t)(1000.0 / syllables_per_second);
    
    // Ограничиваем разумными пределами
    if (cycle_duration_ms < 150) cycle_duration_ms = 150;   // Не быстрее 6.7 Гц
    if (cycle_duration_ms > 800) cycle_duration_ms = 800;   // Не медленнее 1.25 Гц
    
    // Распределение времени в цикле: 60% открыт, 40% закрыт (более естественно)
    uint32_t open_duration = (cycle_duration_ms * 6) / 10;
    uint32_t closed_duration = (cycle_duration_ms * 4) / 10;
    
    printf("[TALKING_NATURAL] Setup: text='%s', syllables=%d, total_duration=%lu ms\n",
           text.c_str(), syllables, total_duration_ms);
    printf("[TALKING_NATURAL] Cycle: %lu ms (open: %lu ms, closed: %lu ms), activity: %.2f\n",
           cycle_duration_ms, open_duration, closed_duration, activity_factor);
    
    // Создаём естественную последовательность с вариациями
    uint32_t accumulated_time = 0;
    int cycle_count = 0;
    
    while (accumulated_time < total_duration_ms) {
        // Добавляем естественные вариации (±20%)
        uint32_t var_open = open_duration + (rand() % (open_duration / 5)) - (open_duration / 10);
        uint32_t var_closed = closed_duration + (rand() % (closed_duration / 5)) - (closed_duration / 10);
        
        // Проверяем, помещается ли полный цикл
        if (accumulated_time + var_open + var_closed <= total_duration_ms) {
            current_animation_sequence.push_back({open_matrix, var_open, "OPEN"});
            current_animation_sequence.push_back({closed_matrix, var_closed, "CLOSED"});
            accumulated_time += var_open + var_closed;
            cycle_count++;
        } else {
            // Последний неполный цикл
            uint32_t remaining = total_duration_ms - accumulated_time;
            if (remaining > 50) { // Минимум 50ms для показа
                current_animation_sequence.push_back({open_matrix, remaining, "FINAL"});
                accumulated_time = total_duration_ms;
            }
            break;
        }
        
        // Иногда добавляем паузы для естественности (каждые 3-4 цикла)
        if (cycle_count % 4 == 0 && accumulated_time + 100 < total_duration_ms) {
            current_animation_sequence.push_back({closed_matrix, 100, "PAUSE"});
            accumulated_time += 100;
        }
    }
    
    current_frame_index = 0;
    frame_start_time = to_ms_since_boot(get_absolute_time());
    
    printf("[TALKING_NATURAL] Created %d frames in %d cycles, planned duration: %lu ms\n",
           (int)current_animation_sequence.size(), cycle_count, accumulated_time);
}

// Обновление естественной анимации (вызывается каждый кадр)
bool update_animation() {
    if (current_animation_sequence.empty()) {
        return false;
    }
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    uint32_t elapsed = current_time - frame_start_time;
    
    if (current_frame_index >= current_animation_sequence.size()) {
        printf("[TALKING_NATURAL] Animation sequence completed (%d frames processed)\n", 
               (int)current_animation_sequence.size());
        return false; // Анимация завершена
    }
    
    const AnimationFrame& current_frame = current_animation_sequence[current_frame_index];
    
    if (elapsed >= current_frame.duration_ms) {
        // Переход к следующему кадру
        current_frame_index++;
        frame_start_time = current_time;
        
        if (current_frame_index < current_animation_sequence.size()) {
            const AnimationFrame& next_frame = current_animation_sequence[current_frame_index];
            draw_matrix(next_frame.matrix, PIXEL_SIZE, false);
            
            // Показываем прогресс каждые 10 кадров для уменьшения спама
            if (current_frame_index % 10 == 0 || current_frame_index < 5) {
                printf("[TALKING_NATURAL] Frame %d/%d: %s (%lu ms)\n",
                       (int)current_frame_index + 1, (int)current_animation_sequence.size(),
                       next_frame.name, next_frame.duration_ms);
            }
        }
        
        return current_frame_index < current_animation_sequence.size();
    } else {
        // Отображаем текущий кадр (только если нужно)
        if (current_frame_index == 0 || animation_dirty) {
            draw_matrix(current_frame.matrix, PIXEL_SIZE, animation_dirty);
            animation_dirty = false;
            if (current_frame_index == 0) {
                printf("[TALKING_NATURAL] Starting first frame: %s (%lu ms)\n",
                       current_frame.name, current_frame.duration_ms);
            }
        }
        return true;
    }
}

int count_syllables(const std::string& text) {
    int count = 0;
    for (char c : text) {
        if (strchr(VOWELS, tolower(c))) {
            count++;
        }
    }
    return count > 0 ? count : 1;
}

// Simple emotion functions
void neutral(double speed, NeutralState& state) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Моргание каждые 3-5 секунд с вариациями
    if (current_time - state.last_blink > (3000 + (rand() % 2000))) {
        if (!state.blink) {
            state.blink = true;
            state.blink_start = current_time;
            state.blink_phase = 0;
        }
        state.last_blink = current_time;
    }
    
    // Зевота каждые 10-15 секунд
    if (current_time - state.last_yawn > (10000 + (rand() % 5000))) {
        draw_matrix(NEUTRAL_YAWN, PIXEL_SIZE, false);
        busy_wait_ms(800); // Короткая пауза вместо sleep_ms
        state.last_yawn = current_time;
        return;
    }
    
    // Плавная анимация моргания
    if (state.blink) {
        uint32_t blink_time = current_time - state.blink_start;
        if (blink_time < 100) {
            draw_matrix(NEUTRAL_HALF_BLINK, PIXEL_SIZE, false);
        } else if (blink_time < 200) {
            draw_matrix(NEUTRAL_BLINK, PIXEL_SIZE, false);
        } else if (blink_time < 300) {
            draw_matrix(NEUTRAL_HALF_BLINK, PIXEL_SIZE, false);
        } else {
            draw_matrix(NEUTRAL_NO_BLINK, PIXEL_SIZE, false);
            state.blink = false;
        }
    } else {
        draw_matrix(NEUTRAL_NO_BLINK, PIXEL_SIZE, false);
    }
}

void smile_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[SMILE] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration, 
               (const uint8_t*)SMILE_B, (const uint8_t*)SMILE_A, 
               (const uint8_t*)SMILE_B, (const uint8_t*)SMILE_A, 
               (const uint8_t*)SMILE);
}

void smile_love_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[SMILE_LOVE] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration,
               (const uint8_t*)SMILE_LOVE, (const uint8_t*)SMILE_LOVE_A,
               (const uint8_t*)SMILE_LOVE_B, (const uint8_t*)SMILE_LOVE_A,
               (const uint8_t*)SMILE);
}

void embarrassed_pixel(double speed, AnimState& state) {
    draw_matrix(EMBARRASSED, PIXEL_SIZE, false);
}

void scary_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[SCARY] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration,
               (const uint8_t*)SCARY_B, (const uint8_t*)SCARY_C,
               (const uint8_t*)SCARY_D, (const uint8_t*)SCARY_C,
               (const uint8_t*)SCARY_A);
}

void happy_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[HAPPY] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration,
               (const uint8_t*)SMILE, (const uint8_t*)SMILE_A,
               (const uint8_t*)SMILE, (const uint8_t*)HAPPY,
               (const uint8_t*)HAPPY);
}

void sad_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[SAD] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration,
               (const uint8_t*)SAD_A, (const uint8_t*)SAD_A,
               (const uint8_t*)SAD, (const uint8_t*)SAD,
               (const uint8_t*)SAD_A);
}

void surprise_pixel(double speed, AnimState& state, uint32_t duration) {
    if (!state.animating) {
        printf("[SURPRISE] Starting animation with speed=%.2f, duration=%lu\n", speed, duration);
    }
    anime_logic(state, speed, duration,
               (const uint8_t*)NEUTRAL_NO_BLINK, (const uint8_t*)SURPRISE,
               (const uint8_t*)SURPRISE, (const uint8_t*)SURPRISE,
               (const uint8_t*)NEUTRAL_NO_BLINK);
}

void talking_pixel(uint32_t duration, double speed, TalkingState& state,
                  const std::string& text, double mouth_speed, const std::string& emotion) {
    
    printf("[TALKING] Called with: duration=%lu, speed=%.2f, mouth_speed=%.2f, emotion='%s', text='%s'\n",
           duration, speed, mouth_speed, emotion.c_str(), text.c_str());
    
    if (emotion == "neutral") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     TALKING_A, TALKING_B, NEUTRAL_NO_BLINK);
    } else if (emotion == "angry") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     ANGRY_OPEN_MOUTH, ANGRY_CLOSED_MOUTH, ANGRY_CLOSED);
    } else if (emotion == "smile_tricky") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     TALKING_TRICKY_A, TALKING_TRICKY_B, SMILE_A);
    } else if (emotion == "tricky") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     SMILE_TRICKY_A, SMILE_TRICKY_B, NEUTRAL_NO_BLINK);
    } else if (emotion == "smile") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     SMILE, TALKING_A, NEUTRAL_NO_BLINK);
    } else if (emotion == "ha") {
        talking_logic(state, text, duration, speed, mouth_speed, 
                     HAPPY_CIRCLE, NEUTRAL_CIRCLE, NEUTRAL_NO_BLINK);
    } else {
        // По умолчанию используем нейтральный разговор
        talking_logic(state, text, duration, speed, mouth_speed, 
                     TALKING_A, TALKING_B, NEUTRAL_NO_BLINK);
    }
}

// Stub functions
void anime_logic(AnimState& state, double speed, uint32_t duration,
                const uint8_t* matrix_start, const uint8_t* matrix_anim_a,
                const uint8_t* matrix_anim_b, const uint8_t* matrix_anim_c,
                const uint8_t* matrix_end) {
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Начинаем анимацию если ещё не начали и duration > 0
    if (!state.animating && duration > 0) {
        state.animating = true;
        state.start_time = current_time;
        state.last_frame = current_time;
        state.frame = 0;
        state.cycle_count = 0;
        
        if (matrix_start) {
            draw_matrix((const uint8_t(*)[MATRIX_COLS])matrix_start, PIXEL_SIZE, true);
        }
        printf("[ANIME_SMOOTH] Starting: duration=%lu ms, speed=%.2f\n", duration, speed);
        return;
    }
    
    // Если анимация не активна и duration <= 2, показываем статичную картинку
    if (!state.animating && duration <= 2) {
        if (matrix_start) {
            draw_matrix((const uint8_t(*)[MATRIX_COLS])matrix_start, PIXEL_SIZE, false);
        }
        return;
    }
    
    // Выполняем анимацию если она активна
    if (state.animating && duration > 2) {
        uint32_t elapsed_time = current_time - state.start_time;
        uint32_t duration_ms = duration * 1000;
        
        if (elapsed_time < duration_ms) {
            // Рассчитываем длительность кадра с правильными типами
            uint32_t frame_duration = (uint32_t)(speed * 1000);
            if (frame_duration < 100) {
                frame_duration = 100; // Минимум 100ms
            }
            
            if (current_time - state.last_frame >= frame_duration) {
                state.frame = (state.frame + 1) % 4;
                state.cycle_count = elapsed_time / (frame_duration * 2);
                
                const uint8_t* current_matrix = nullptr;
                
                switch (state.frame) {
                    case 0: current_matrix = matrix_start; break;
                    case 1: current_matrix = matrix_anim_a; break;
                    case 2: current_matrix = matrix_anim_b; break;
                    case 3: current_matrix = matrix_anim_c; break;
                }
                
                if (current_matrix) {
                    draw_matrix((const uint8_t(*)[MATRIX_COLS])current_matrix, PIXEL_SIZE, false);
                }
                
                state.last_frame = current_time;
            }
        } else {
            // Завершение анимации
            state.animating = false;
            if (matrix_end) {
                draw_matrix((const uint8_t(*)[MATRIX_COLS])matrix_end, PIXEL_SIZE, true);
            }
            printf("[ANIME_SMOOTH] Animation completed\n");
        }
    }
}

void talking_logic(TalkingState& state, const std::string& text, uint32_t duration,
                  double speed, double mouth_speed,
                  const uint8_t open_matrix[MATRIX_ROWS][MATRIX_COLS],
                  const uint8_t closed_matrix[MATRIX_ROWS][MATRIX_COLS],
                  const uint8_t neutral_matrix[MATRIX_ROWS][MATRIX_COLS]) {
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Начинаем новую анимацию разговора с улучшенной синхронизацией
    if (!text.empty() && !state.talking) {
        state.talking = true;
        state.start_time = current_time;
        
        printf("[TALKING_NATURAL] Starting natural speech: '%s'\n", text.c_str());
        printf("[TALKING_NATURAL] Duration: %lu ms, mouth_speed: %.2f (lower=faster movement)\n", 
               duration * 1000, mouth_speed);
        
        // Настраиваем естественную систему анимации
        setup_talking_animation(text, duration * 1000, mouth_speed, open_matrix, closed_matrix);
        animation_dirty = true;
        return;
    }
    
    if (state.talking) {
        uint32_t speech_duration = duration * 1000;
        uint32_t elapsed_time = current_time - state.start_time;
        
        if (elapsed_time < speech_duration) {
            // Обновляем естественную анимацию без блокировок
            bool animation_active = update_animation();
            
            if (!animation_active) {
                // Анимация завершена раньше времени - показываем нейтральное лицо
                printf("[TALKING_NATURAL] Animation sequence completed early, showing neutral\n");
                draw_matrix(neutral_matrix, PIXEL_SIZE, false);
            }
        } else {
            // Завершение разговора
            state.talking = false;
            current_animation_sequence.clear();
            draw_matrix(neutral_matrix, PIXEL_SIZE, true);
            printf("[TALKING_NATURAL] Natural speech completed: '%s'\n", text.c_str());
        }
    } else {
        // Показываем нейтральное лицо когда не разговариваем
        draw_matrix(neutral_matrix, PIXEL_SIZE, false);
    }
}
