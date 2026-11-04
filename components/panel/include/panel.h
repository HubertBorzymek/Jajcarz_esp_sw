// Model panelu: stany klawiszy i ich kolory. Czysta logika, bez sprzętu i RTOS.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "jajcarz_types.h"

#define PANEL_MAX_KEYS 32

typedef enum {
    PANEL_STATE_OK = 0,
    PANEL_STATE_LOW,
    PANEL_STATE_OUT,
    PANEL_STATE_COUNT
} panel_state_t;

typedef enum {
    PANEL_PRESS_STATE_CHANGED,  // klawisz przypisany, stan przeszedł dalej
    PANEL_PRESS_UNASSIGNED,     // klawisz bez produktu, stan bez zmian
} panel_press_t;

typedef struct {
    key_addr_t    addr;
    panel_state_t state;
    bool          assigned;
} panel_key_t;

typedef struct {
    uint8_t brightness;                 // globalna jasność 0..255
    rgb_t   colors[PANEL_STATE_COUNT];
} panel_config_t;

typedef struct {
    panel_key_t    keys[PANEL_MAX_KEYS];
    size_t         key_count;
    panel_config_t config;
} panel_t;

// Wszystkie klawisze startują jako przypisane w stanie OK. Serwer zmieni
// przypisania po pierwszym połączeniu, inaczej panel do tego czasu byłby ciemny.
void panel_init(panel_t *panel, const key_addr_t *addrs, size_t count);

panel_press_t panel_press(panel_t *panel, size_t index);
bool panel_set_state(panel_t *panel, size_t index, panel_state_t state);
bool panel_set_assigned(panel_t *panel, size_t index, bool assigned);

// Kolor do wysłania na diodę, z uwzględnieniem jasności i przypisania.
rgb_t panel_led_color(const panel_t *panel, size_t index);

const char *panel_state_name(panel_state_t state);
