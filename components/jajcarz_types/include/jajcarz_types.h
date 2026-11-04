// Typy wspólne dla modułów, bez zależności od sprzętu.
#pragma once

#include <stdint.h>

typedef struct {
    uint8_t r, g, b;
} rgb_t;

// Adres klawisza "segment.klawisz", np. 0.2. Patrz docs/PROTOCOL.md.
typedef struct {
    uint8_t seg;
    uint8_t key;
} key_addr_t;
