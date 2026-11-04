// Interfejs sterownika segmentu. Nowy sposób podłączenia modułu, np. ekspander
// I²C, to nowa implementacja tej struktury wpisana do tablicy w segments.c.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "jajcarz_types.h"

typedef struct {
    uint8_t id;
    uint8_t key_count;
    esp_err_t (*init)(void);
    bool (*key_pressed)(uint8_t key);
    void (*set_led)(uint8_t key, rgb_t color);
    esp_err_t (*show)(void);
} segment_driver_t;

extern const segment_driver_t segment_local_driver;
