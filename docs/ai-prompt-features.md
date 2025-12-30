# AI Промпт: Добавление новых функций

## 🚀 Расширение функциональности Robot Pico

Вы помогаете добавить новую функцию в embedded проект Robot Pico. Следуйте принципам модульной разработки и embedded ограничениям.

### 📋 Алгоритм добавления функций

#### 1. Анализ требований
- ✅ Совместимость с RP2040 (264KB RAM, 2MB Flash)
- ✅ Соответствие реальному времени (1ms цикл)
- ✅ Минимальное влияние на существующий код
- ✅ Возможность отключения для экономии ресурсов

#### 2. Планирование архитектуры
- ✅ Определить модуль (core/display/emotions/новый)
- ✅ Спроектировать API интерфейс
- ✅ Выбрать используемые пины GPIO
- ✅ Оценить влияние на производительность

#### 3. Поэтапная реализация
- ✅ Создать минимальную версию (MVP)
- ✅ Интегрировать с основным циклом
- ✅ Добавить конфигурацию и настройки
- ✅ Реализовать полный функционал

### 🎵 Пример: Добавление звука (Buzzer)

#### Этап 1: Заголовочный файл
```cpp
// include/audio/buzzer.h
#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Конфигурация
#define BUZZER_PIN 15
#define BUZZER_PWM_SLICE pwm_gpio_to_slice_num(BUZZER_PIN)

// Ноты (частоты в Hz)
typedef enum {
    NOTE_SILENT = 0,
    NOTE_C4 = 262,
    NOTE_D4 = 294,
    NOTE_E4 = 330,
    NOTE_F4 = 349,
    NOTE_G4 = 392,
    NOTE_A4 = 440,
    NOTE_B4 = 494
} buzzer_note_t;

// Структура состояния
typedef struct {
    bool enabled;
    bool playing;
    uint32_t start_time;
    uint32_t duration_ms;
    buzzer_note_t current_note;
} buzzer_state_t;

// API функции
void buzzer_init(void);
void buzzer_play_note(buzzer_note_t note, uint32_t duration_ms);
void buzzer_play_melody(const buzzer_note_t* notes, const uint32_t* durations, int count);
void buzzer_stop(void);
void buzzer_update(void);  // Вызывается в main loop
bool buzzer_is_playing(void);

// Предустановленные мелодии
void buzzer_play_startup_sound(void);
void buzzer_play_notification(void);
void buzzer_play_error(void);

#endif // BUZZER_H
```

#### Этап 2: Реализация
```cpp
// src/audio/buzzer.c
#include "buzzer.h"

static buzzer_state_t buzzer_state = {0};
static uint slice_num;

void buzzer_init(void) {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    
    // Настройка PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 8.f);  // Делитель частоты
    pwm_init(slice_num, &config, true);
    
    buzzer_state.enabled = true;
    buzzer_stop();
    
    printf("[BUZZER] Initialized on pin %d\n", BUZZER_PIN);
}

void buzzer_play_note(buzzer_note_t note, uint32_t duration_ms) {
    if (!buzzer_state.enabled) return;
    
    buzzer_state.current_note = note;
    buzzer_state.duration_ms = duration_ms;
    buzzer_state.start_time = to_ms_since_boot(get_absolute_time());
    buzzer_state.playing = true;
    
    if (note == NOTE_SILENT) {
        pwm_set_gpio_level(BUZZER_PIN, 0);
    } else {
        // Установка частоты PWM для ноты
        uint32_t clock_freq = 125000000;  // 125 MHz
        uint32_t divider = 8;
        uint32_t pwm_freq = clock_freq / divider;
        uint16_t wrap = pwm_freq / note - 1;
        
        pwm_set_wrap(slice_num, wrap);
        pwm_set_gpio_level(BUZZER_PIN, wrap / 2);  // 50% duty cycle
    }
}

void buzzer_update(void) {
    if (!buzzer_state.playing) return;
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - buzzer_state.start_time >= buzzer_state.duration_ms) {
        buzzer_stop();
    }
}

void buzzer_stop(void) {
    pwm_set_gpio_level(BUZZER_PIN, 0);
    buzzer_state.playing = false;
    buzzer_state.current_note = NOTE_SILENT;
}

// Предустановленные мелодии
void buzzer_play_startup_sound(void) {
    static const buzzer_note_t notes[] = {NOTE_C4, NOTE_E4, NOTE_G4};
    static const uint32_t durations[] = {200, 200, 400};
    buzzer_play_melody(notes, durations, 3);
}
```

#### Этап 3: Интеграция в main.cpp
```cpp
// В main.cpp добавить:

#include "buzzer.h"  // Новый заголовок

// В main() после инициализации дисплея:
buzzer_init();
buzzer_play_startup_sound();

// В основном цикле:
while (true) {
    // ... существующий код ...
    
    buzzer_update();  // Обновление звука
    
    sleep_ms(1);  // 1ms цикл
}

// В обработке команд JSON:
if (command.find("sound") != command.end()) {
    std::string sound = command["sound"];
    if (sound == "notification") {
        buzzer_play_notification();
    } else if (sound == "error") {
        buzzer_play_error();
    }
}
```

### 📡 Пример: Добавление Wi-Fi (Pico W)

#### Архитектура модуля:
```
src/network/
├── wifi_manager.cpp    # Управление Wi-Fi подключением
├── web_server.cpp      # HTTP сервер для команд
└── mqtt_client.cpp     # MQTT клиент (опционально)

include/network/
├── wifi_manager.h
├── web_server.h
└── mqtt_client.h
```

#### API интерфейс:
```cpp
// include/network/wifi_manager.h
typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_ERROR
} wifi_status_t;

void wifi_init(const char* ssid, const char* password);
wifi_status_t wifi_get_status(void);
void wifi_update(void);  // Неблокирующая проверка состояния
const char* wifi_get_ip(void);
```

### 🎮 Пример: Добавление джойстика

#### Конфигурация GPIO:
```cpp
// include/input/joystick.h
#define JOYSTICK_X_PIN    26  // ADC0
#define JOYSTICK_Y_PIN    27  // ADC1
#define JOYSTICK_BTN_PIN  16  // Digital

typedef struct {
    int16_t x;        // -1000 to +1000
    int16_t y;        // -1000 to +1000
    bool button;      // true = pressed
    bool changed;     // true = state changed
} joystick_state_t;
```

#### Интеграция с эмоциями:
```cpp
// В main.cpp
joystick_state_t joystick = joystick_read();

if (joystick.changed) {
    if (joystick.x > 500) {
        current_emotion = "happy";
    } else if (joystick.x < -500) {
        current_emotion = "sad";
    } else if (joystick.y > 500) {
        current_emotion = "surprise";
    } else if (joystick.button) {
        current_emotion = "smile";
    }
}
```

### 🌡️ Пример: Добавление датчиков

#### Модульная архитектура:
```cpp
// include/sensors/sensor_manager.h
typedef struct {
    float temperature;    // °C
    float humidity;       // %
    uint16_t light_level; // 0-1023
    bool motion_detected; // PIR sensor
    uint32_t timestamp;   // ms
} sensor_data_t;

// Неблокирующий API
void sensors_init(void);
void sensors_update(void);          // Вызывается каждый цикл
sensor_data_t sensors_get_data(void);
bool sensors_data_ready(void);      // Новые данные доступны
```

#### Реакция на датчики:
```cpp
// Автоматическое переключение эмоций на основе датчиков
void update_emotion_from_sensors() {
    sensor_data_t sensors = sensors_get_data();
    
    if (sensors.motion_detected) {
        current_emotion = "surprise";
        emotion_timer = get_time();
    } else if (sensors.temperature > 30.0f) {
        current_emotion = "embarrassed";  // "жарко"
    } else if (sensors.light_level < 100) {
        current_emotion = "sleep";        // "темно"
    }
}
```

### 🔧 Шаблон добавления модуля

#### 1. Создать файловую структуру:
```bash
mkdir src/new_module
mkdir include/new_module
touch src/new_module/new_module.cpp
touch include/new_module/new_module.h
```

#### 2. Базовый заголовочный файл:
```cpp
#ifndef NEW_MODULE_H
#define NEW_MODULE_H

#include "pico/stdlib.h"

// Конфигурация
#define MODULE_ENABLED 1

// Состояние модуля
typedef struct {
    bool initialized;
    bool enabled;
    uint32_t last_update;
} module_state_t;

// API
void module_init(void);
void module_update(void);    // Неблокирующая
void module_enable(bool enable);
bool module_is_ready(void);

#endif
```

#### 3. Базовая реализация:
```cpp
#include "new_module.h"

static module_state_t state = {0};

void module_init(void) {
    if (state.initialized) return;
    
    // Инициализация GPIO, периферии
    
    state.initialized = true;
    state.enabled = true;
    state.last_update = to_ms_since_boot(get_absolute_time());
    
    printf("[MODULE] Initialized\n");
}

void module_update(void) {
    if (!state.enabled || !state.initialized) return;
    
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Неблокирующая логика обновления
    
    state.last_update = current_time;
}
```

### 📝 Чек-лист интеграции

#### Перед добавлением:
- ✅ Проверить доступность GPIO пинов
- ✅ Оценить потребление памяти
- ✅ Убедиться в совместимости с 1ms циклом
- ✅ Спланировать конфигурационные опции

#### После добавления:
- ✅ Протестировать на реальном оборудовании
- ✅ Проверить влияние на производительность анимаций
- ✅ Обновить CMakeLists.txt
- ✅ Добавить документацию в README.md
- ✅ Создать примеры команд JSON

#### Добавить в CMakeLists.txt:
```cmake
# Новый модуль
target_include_directories(robot_pico PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include/new_module
)

# Дополнительные библиотеки (если нужно)
target_link_libraries(robot_pico
    hardware_adc      # Для аналоговых датчиков
    hardware_pwm      # Для buzzer/servo
    hardware_i2c      # Для I2C датчиков
)
```

### 🎯 Приоритеты функций

#### Высокий приоритет (легко реализовать):
1. **Buzzer** - звуковые эффекты
2. **LED индикаторы** - статусные светодиоды
3. **Кнопки** - физическое управление
4. **Потенциометры** - аналоговые настройки

#### Средний приоритет (требует планирования):
1. **Датчики температуры/влажности** (DHT22, SHT30)
2. **Датчик движения** (PIR)
3. **Фотодатчик** - реакция на освещение
4. **Серво моторы** - физические движения

#### Низкий приоритет (сложная интеграция):
1. **Wi-Fi/Bluetooth** - беспроводная связь
2. **SD карта** - хранение данных
3. **Камера** - компьютерное зрение
4. **Голосовой синтез** - TTS

---

**Принцип:** Начинай с простого, тестируй на каждом шаге, сохраняй производительность!