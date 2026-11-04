// Dostęp do klawiszy i diod wszystkich segmentów, niezależnie od ich podłączenia.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "jajcarz_types.h"

typedef struct {
    uint8_t id;         // adres segmentu, część adresu klawisza
    uint8_t key_count;
} segment_info_t;

esp_err_t segments_init(void);

size_t segments_count(void);
segment_info_t segments_info(size_t index);

// true = klawisz fizycznie naciśnięty, odczyt bez debouncingu.
bool segments_key_pressed(key_addr_t addr);

void segments_set_led(key_addr_t addr, rgb_t color);

// Wysyła kolory na diody. Bez zmian od poprzedniego wywołania nic nie robi.
esp_err_t segments_show(void);
