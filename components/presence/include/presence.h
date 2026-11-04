// Tryby obecności zależne od odległości użytkownika. Bez czujnika zawsze ACTIVE.
#pragma once

#include <stdbool.h>

typedef enum {
    PRESENCE_SLEEP,     // nikogo w pobliżu: diody zgaszone, klawisze ignorowane
    PRESENCE_DISPLAY,   // średni dystans: diody świecą, klawisze ignorowane
    PRESENCE_ACTIVE,    // blisko: pełna obsługa
} presence_mode_t;

presence_mode_t presence_get_mode(void);

bool presence_leds_enabled(presence_mode_t mode);
bool presence_keys_enabled(presence_mode_t mode);
