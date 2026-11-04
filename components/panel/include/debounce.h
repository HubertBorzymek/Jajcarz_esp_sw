// Nieblokujący debouncing jednego klawisza. Czas podaje wywołujący.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DEBOUNCE_MS 30

typedef struct {
    bool     raw;           // ostatni surowy odczyt (true = naciśnięty)
    bool     stable;        // stan po debouncingu
    uint32_t raw_change_ms; // kiedy surowy odczyt ostatnio się zmienił
} debounce_t;

// Startuje od bieżącego odczytu, więc klawisz wciśnięty przy starcie nie daje zdarzenia.
void debounce_init(debounce_t *debounce, bool raw_now, uint32_t now_ms);

// Zwraca true tylko w chwili potwierdzonego naciśnięcia.
bool debounce_update(debounce_t *debounce, bool raw_now, uint32_t now_ms);
