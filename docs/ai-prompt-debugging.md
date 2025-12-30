# AI Промпт: Отладка и диагностика

## 🐛 Debugging Robot Pico - Embedded систем

Вы помогаете диагностировать и исправлять проблемы в embedded проекте на RP2040. Используйте систематический подход и embedded-специфичные техники отладки.

### 🎯 Классификация проблем

#### 🔴 Критические (система не работает):
- Не компилируется
- Не загружается на Pico
- Зависает при инициализации
- Постоянные перезагрузки (watchdog reset)

#### 🟡 Функциональные (частичная работа):
- Дисплей не отображает / мерцает
- Команды не принимаются
- Анимации тормозят/прерываются
- USB связь нестабильна

#### 🟢 Производительность (работает медленно):
- Низкий FPS анимаций
- Задержки реакции на команды
- Высокое потребление CPU/памяти

### 🔧 Диагностические инструменты

#### 1. Отладочный вывод
```cpp
// Уровни отладки
#define DEBUG_LEVEL_ERROR   1
#define DEBUG_LEVEL_WARNING 2  
#define DEBUG_LEVEL_INFO    3
#define DEBUG_LEVEL_DEBUG   4

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

// Макросы для отладки
#define DEBUG_ERROR(fmt, ...)   if(DEBUG_LEVEL >= 1) printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
#define DEBUG_WARNING(fmt, ...) if(DEBUG_LEVEL >= 2) printf("[WARN]  " fmt "\n", ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    if(DEBUG_LEVEL >= 3) printf("[INFO]  " fmt "\n", ##__VA_ARGS__)
#define DEBUG_TRACE(fmt, ...)   if(DEBUG_LEVEL >= 4) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)

// Использование
void problematic_function() {
    DEBUG_TRACE("Entering function with param=%d", some_param);
    
    if (error_condition) {
        DEBUG_ERROR("Critical error: %s", error_message);
        return;
    }
    
    DEBUG_INFO("Function completed successfully");
}
```

#### 2. Профилирование времени выполнения
```cpp
// Измерение производительности функций
#define PROFILE_FUNCTION_START() \
    uint32_t __prof_start = to_us_since_boot(get_absolute_time())

#define PROFILE_FUNCTION_END(name) \
    uint32_t __prof_duration = to_us_since_boot(get_absolute_time()) - __prof_start; \
    if (__prof_duration > 1000) { \
        DEBUG_WARNING("SLOW: %s took %lu us", name, __prof_duration); \
    }

void animation_update() {
    PROFILE_FUNCTION_START();
    
    // Код анимации
    
    PROFILE_FUNCTION_END("animation_update");
}
```

#### 3. Мониторинг состояния системы
```cpp
typedef struct {
    uint32_t free_ram;
    uint32_t min_free_ram;
    uint32_t cpu_usage_percent;
    uint32_t frame_time_us;
    uint32_t max_frame_time_us;
    uint32_t loop_counter;
} system_stats_t;

static system_stats_t sys_stats = {0};

void update_system_stats() {
    // Измерение свободной RAM
    extern char __HeapBase, __HeapLimit;
    sys_stats.free_ram = &__HeapLimit - &__HeapBase - malloc_usable_size(NULL);
    
    if (sys_stats.free_ram < sys_stats.min_free_ram) {
        sys_stats.min_free_ram = sys_stats.free_ram;
    }
    
    // Счетчик циклов
    sys_stats.loop_counter++;
}

void print_system_stats() {
    DEBUG_INFO("RAM: %lu KB free (%lu min), CPU: %lu%%, Frame: %lu us (max %lu)", 
               sys_stats.free_ram / 1024,
               sys_stats.min_free_ram / 1024,
               sys_stats.cpu_usage_percent,
               sys_stats.frame_time_us,
               sys_stats.max_frame_time_us);
}
```

### 🚨 Типичные проблемы и решения

#### 1. Проблемы компиляции

##### Ошибка: "undefined reference to..."
```bash
# Диагностика
arm-none-eabi-nm robot_pico.elf | grep missing_symbol

# Решения:
# 1. Добавить библиотеку в CMakeLists.txt
target_link_libraries(robot_pico missing_library)

# 2. Проверить объявления функций
grep -r "function_name" include/

# 3. Проверить extern "C" для C библиотек
extern "C" {
#include "c_library.h"
}
```

##### Ошибка: "multiple definition of..."
```cpp
// Проблема: глобальная переменная в заголовке
// header.h - НЕПРАВИЛЬНО
int global_var = 42;

// header.h - ПРАВИЛЬНО
extern int global_var;

// source.cpp
int global_var = 42;
```

#### 2. Проблемы с дисплеем

##### Черный экран:
```cpp
// Диагностика подключения
void test_display_pins() {
    DEBUG_INFO("Testing display pins...");
    
    // Проверка SPI
    gpio_put(DISPLAY_CS_PIN, 0);
    spi_write_blocking(DISPLAY_SPI_PORT, (uint8_t[]){0x00}, 1);
    gpio_put(DISPLAY_CS_PIN, 1);
    
    // Проверка DC pin
    gpio_put(DISPLAY_DC_PIN, 1);
    sleep_ms(1);
    gpio_put(DISPLAY_DC_PIN, 0);
    
    DEBUG_INFO("Pin test completed");
}

// Пошаговая инициализация
void debug_display_init() {
    DEBUG_INFO("Step 1: GPIO init");
    // Инициализация GPIO
    
    DEBUG_INFO("Step 2: SPI init");  
    // Инициализация SPI
    
    DEBUG_INFO("Step 3: Reset sequence");
    // Reset дисплея
    
    DEBUG_INFO("Step 4: ST7789 init");
    // Инициализация ST7789
    
    DEBUG_INFO("Step 5: Test pattern");
    // Тестовый паттерн
}
```

##### Мерцание дисплея:
```cpp
// Анализ производительности отрисовки
static uint32_t draw_start_time = 0;

void debug_draw_performance() {
    draw_start_time = to_us_since_boot(get_absolute_time());
}

void debug_draw_end() {
    uint32_t draw_time = to_us_since_boot(get_absolute_time()) - draw_start_time;
    
    if (draw_time > 16670) {  // > 16.67ms = < 60 FPS
        DEBUG_WARNING("Slow draw: %lu us (target: 16670 us)", draw_time);
    }
}

// Проверка на лишние перерисовки
static uint32_t draw_count = 0;
static uint32_t last_fps_check = 0;

void monitor_draw_calls() {
    draw_count++;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_fps_check >= 1000) {  // Каждую секунду
        if (draw_count > 70) {  // Больше 60 FPS
            DEBUG_WARNING("Too many redraws: %lu FPS", draw_count);
        }
        draw_count = 0;
        last_fps_check = current_time;
    }
}
```

#### 3. Проблемы с USB/Serial

##### Команды не принимаются:
```cpp
void debug_serial_communication() {
    DEBUG_INFO("Serial debug mode enabled");
    
    while (true) {
        int c = getchar_timeout_us(100000);  // 100ms timeout
        
        if (c == PICO_ERROR_TIMEOUT) {
            DEBUG_TRACE("No serial data received");
            continue;
        }
        
        DEBUG_INFO("Received byte: 0x%02X ('%c')", c, 
                   (c >= 32 && c <= 126) ? c : '.');
        
        // Эхо обратно
        putchar(c);
    }
}

// Анализ JSON парсинга
void debug_json_parsing(const std::string& json_str) {
    DEBUG_INFO("Parsing JSON: '%s'", json_str.c_str());
    DEBUG_INFO("Length: %d bytes", json_str.length());
    
    // Проверка на валидные символы
    for (size_t i = 0; i < json_str.length(); i++) {
        char c = json_str[i];
        if (c < 32 || c > 126) {
            DEBUG_WARNING("Invalid character at pos %d: 0x%02X", i, c);
        }
    }
    
    auto result = parse_json(json_str);
    DEBUG_INFO("Parsed %d fields:", result.size());
    
    for (const auto& pair : result) {
        DEBUG_INFO("  '%s' = '%s'", pair.first.c_str(), pair.second.c_str());
    }
}
```

#### 4. Проблемы с памятью

##### Утечки памяти:
```cpp
// Мониторинг использования heap
void monitor_heap_usage() {
    static uint32_t prev_free = 0;
    uint32_t current_free = get_free_heap_size();
    
    if (prev_free > 0 && current_free < prev_free) {
        int32_t diff = prev_free - current_free;
        if (diff > 1024) {  // Больше 1KB
            DEBUG_WARNING("Heap usage increased by %ld bytes", diff);
        }
    }
    
    prev_free = current_free;
}

// Проверка переполнения стека
void check_stack_overflow() {
    extern char __StackLimit;
    volatile char stack_var;
    
    ptrdiff_t stack_used = &stack_var - &__StackLimit;
    
    if (stack_used > 32768) {  // Больше 32KB
        DEBUG_ERROR("Stack overflow risk: %td bytes used", stack_used);
    }
}
```

##### Фрагментация памяти:
```cpp
// Анализ фрагментации
void analyze_heap_fragmentation() {
    const size_t test_sizes[] = {32, 64, 128, 256, 512, 1024, 2048};
    
    for (size_t i = 0; i < sizeof(test_sizes)/sizeof(test_sizes[0]); i++) {
        void* ptr = malloc(test_sizes[i]);
        if (ptr) {
            free(ptr);
            DEBUG_INFO("Can allocate %d bytes", test_sizes[i]);
        } else {
            DEBUG_WARNING("Cannot allocate %d bytes", test_sizes[i]);
            break;
        }
    }
}
```

### 🔍 Отладка в реальном времени

#### 1. LED индикаторы состояния
```cpp
// Визуальная диагностика через встроенный LED
#define LED_PIN PICO_DEFAULT_LED_PIN

typedef enum {
    LED_PATTERN_OFF,          // Выключен
    LED_PATTERN_SLOW_BLINK,   // Медленное мигание - норма
    LED_PATTERN_FAST_BLINK,   // Быстрое мигание - предупреждение  
    LED_PATTERN_SOLID,        // Постоянно - ошибка
    LED_PATTERN_HEARTBEAT     // Двойное мигание - жив
} led_pattern_t;

void set_debug_led_pattern(led_pattern_t pattern) {
    static led_pattern_t current_pattern = LED_PATTERN_OFF;
    static uint32_t last_toggle = 0;
    static bool led_state = false;
    
    current_pattern = pattern;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    switch (current_pattern) {
        case LED_PATTERN_SLOW_BLINK:
            if (current_time - last_toggle >= 1000) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                last_toggle = current_time;
            }
            break;
            
        case LED_PATTERN_FAST_BLINK:
            if (current_time - last_toggle >= 200) {
                led_state = !led_state;
                gpio_put(LED_PIN, led_state);
                last_toggle = current_time;
            }
            break;
            
        case LED_PATTERN_SOLID:
            gpio_put(LED_PIN, true);
            break;
            
        default:
            gpio_put(LED_PIN, false);
            break;
    }
}
```

#### 2. Watchdog для обнаружения зависаний
```cpp
#include "hardware/watchdog.h"

#define WATCHDOG_TIMEOUT_MS 5000  // 5 секунд

void setup_watchdog() {
    if (watchdog_caused_reboot()) {
        DEBUG_ERROR("System recovered from watchdog reset!");
    }
    
    watchdog_enable(WATCHDOG_TIMEOUT_MS, true);
    DEBUG_INFO("Watchdog enabled (%d ms timeout)", WATCHDOG_TIMEOUT_MS);
}

void main_loop() {
    while (true) {
        watchdog_update();  // Сброс watchdog каждый цикл
        
        // Основная логика программы
        
        sleep_ms(1);
    }
}
```

### 📊 Автоматическая диагностика

#### 1. Self-test при запуске
```cpp
typedef struct {
    bool display_ok;
    bool spi_ok;
    bool memory_ok;
    bool timing_ok;
} selftest_result_t;

selftest_result_t run_selftest() {
    selftest_result_t result = {0};
    
    DEBUG_INFO("Starting self-test...");
    
    // Тест дисплея
    DEBUG_INFO("Testing display...");
    st7789_fill(0xF800);  // Красный
    sleep_ms(100);
    st7789_fill(0x0000);  // Черный
    result.display_ok = true;  // Если не зависло
    
    // Тест памяти
    DEBUG_INFO("Testing memory...");
    void* test_ptr = malloc(1024);
    if (test_ptr) {
        memset(test_ptr, 0xAA, 1024);
        result.memory_ok = true;
        free(test_ptr);
    }
    
    // Тест тайминга
    DEBUG_INFO("Testing timing...");
    uint32_t start = to_ms_since_boot(get_absolute_time());
    sleep_ms(100);
    uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - start;
    result.timing_ok = (elapsed >= 90 && elapsed <= 110);  // ±10ms точность
    
    DEBUG_INFO("Self-test completed: Display=%s, Memory=%s, Timing=%s",
               result.display_ok ? "OK" : "FAIL",
               result.memory_ok ? "OK" : "FAIL", 
               result.timing_ok ? "OK" : "FAIL");
    
    return result;
}
```

#### 2. Непрерывный мониторинг
```cpp
void health_monitor() {
    static uint32_t last_check = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    if (current_time - last_check >= 5000) {  // Каждые 5 секунд
        
        // Проверка свободной памяти
        uint32_t free_ram = get_free_heap_size();
        if (free_ram < 10240) {  // Меньше 10KB
            DEBUG_ERROR("Low memory warning: %lu bytes free", free_ram);
            set_debug_led_pattern(LED_PATTERN_FAST_BLINK);
        }
        
        // Проверка производительности
        if (sys_stats.frame_time_us > 20000) {  // Больше 20ms
            DEBUG_WARNING("Performance warning: frame time %lu us", 
                         sys_stats.frame_time_us);
        }
        
        last_check = current_time;
    }
}
```

### 🛠️ Инструменты внешней диагностики

#### 1. Serial log analyzer (Python скрипт)
```python
#!/usr/bin/env python3
import serial
import re
import time

def analyze_logs(port='/dev/ttyACM0', baud=115200):
    ser = serial.Serial(port, baud, timeout=1)
    
    patterns = {
        'error': re.compile(r'\[ERROR\](.*)'),
        'warning': re.compile(r'\[WARN\](.*)'),  
        'performance': re.compile(r'took (\d+) us'),
        'memory': re.compile(r'(\d+) KB free')
    }
    
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        
        for pattern_name, pattern in patterns.items():
            match = pattern.search(line)
            if match:
                print(f"[{pattern_name.upper()}] {match.group(1)}")
```

#### 2. Memory map analysis
```bash
# Анализ использования Flash памяти
arm-none-eabi-size -A robot_pico.elf

# Топ функций по размеру
arm-none-eabi-nm -S robot_pico.elf | sort -k2 -n | tail -20

# Анализ стека вызовов
arm-none-eabi-objdump -S robot_pico.elf > disassembly.txt
```

---

**Методология отладки:** Воспроизведи → Изолируй → Исправь → Проверь → Документируй