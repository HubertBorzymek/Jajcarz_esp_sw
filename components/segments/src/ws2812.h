// Sterownik łańcucha WS2812B (GRB, 800 kHz) na RMT.
#pragma once

#include <stddef.h>
#include "driver/gpio.h"
#include "esp_err.h"
#include "jajcarz_types.h"

// 128 symboli pamięci RMT mieści cały łańcuch, więc transmisja nie zależy od przerwań.
#define WS2812_MAX_PIXELS 5

esp_err_t ws2812_init(gpio_num_t gpio);

// Blokuje do końca transmisji. Niska linia po wysyłce działa jak reset/latch.
esp_err_t ws2812_write(const rgb_t *pixels, size_t count);
