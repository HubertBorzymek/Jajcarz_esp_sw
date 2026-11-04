#include "panel.h"

#include <string.h>

#define IDENTIFY_PERIOD_MS 300

static const rgb_t COLOR_OFF = { 0, 0, 0 };
static const rgb_t COLOR_IDENTIFY = { 255, 255, 255 };

static const panel_config_t DEFAULT_CONFIG = {
    .brightness = 51,   // ok. 20%
    .colors = {
        [PANEL_STATE_OK]  = { 0,   255, 0 },    // zielony
        [PANEL_STATE_LOW] = { 255, 120, 0 },    // żółty, mniej G, żeby nie wpadał w zieleń
        [PANEL_STATE_OUT] = { 255, 0,   0 },    // czerwony
    },
};

static const char *const STATE_NAMES[PANEL_STATE_COUNT] = {
    [PANEL_STATE_OK]  = "OK",
    [PANEL_STATE_LOW] = "LOW",
    [PANEL_STATE_OUT] = "OUT",
};

static uint8_t scale(uint8_t value, uint8_t brightness)
{
    return (uint8_t)(((uint16_t)value * brightness) / 255);
}

static rgb_t apply_brightness(rgb_t color, uint8_t brightness)
{
    return (rgb_t){ scale(color.r, brightness), scale(color.g, brightness), scale(color.b, brightness) };
}

static bool identify_active(const panel_t *panel, size_t index, uint32_t now_ms)
{
    return index == panel->identify_index && (now_ms - panel->identify_start_ms) < panel->identify_duration_ms;
}

void panel_init(panel_t *panel, const key_addr_t *addrs, size_t count)
{
    panel->key_count = count < PANEL_MAX_KEYS ? count : PANEL_MAX_KEYS;
    panel->config = DEFAULT_CONFIG;
    panel->identify_duration_ms = 0;
    for (size_t i = 0; i < panel->key_count; i++) {
        panel->keys[i] = (panel_key_t){
            .addr = addrs[i],
            .state = PANEL_STATE_OK,
            .assigned = true,
        };
    }
}

panel_press_t panel_press(panel_t *panel, size_t index)
{
    panel_key_t *key = &panel->keys[index];
    if (!key->assigned) {
        return PANEL_PRESS_UNASSIGNED;
    }
    key->state = (panel_state_t)((key->state + 1) % PANEL_STATE_COUNT);
    return PANEL_PRESS_STATE_CHANGED;
}

bool panel_set_state(panel_t *panel, size_t index, panel_state_t state)
{
    if (index >= panel->key_count || state >= PANEL_STATE_COUNT) {
        return false;
    }
    panel->keys[index].state = state;
    return true;
}

bool panel_set_assigned(panel_t *panel, size_t index, bool assigned)
{
    if (index >= panel->key_count) {
        return false;
    }
    panel->keys[index].assigned = assigned;
    return true;
}

void panel_identify(panel_t *panel, size_t index, uint32_t duration_ms, uint32_t now_ms)
{
    panel->identify_index = index;
    panel->identify_start_ms = now_ms;
    panel->identify_duration_ms = duration_ms;
}

rgb_t panel_led_color(const panel_t *panel, size_t index, uint32_t now_ms)
{
    uint8_t brightness = panel->config.brightness;
    if (identify_active(panel, index, now_ms)) {
        bool lit = ((now_ms - panel->identify_start_ms) % IDENTIFY_PERIOD_MS) < (IDENTIFY_PERIOD_MS / 2);
        return lit ? apply_brightness(COLOR_IDENTIFY, brightness) : COLOR_OFF;
    }
    const panel_key_t *key = &panel->keys[index];
    if (!key->assigned) {
        return COLOR_OFF;
    }
    return apply_brightness(panel->config.colors[key->state], brightness);
}

bool panel_find(const panel_t *panel, key_addr_t addr, size_t *index)
{
    for (size_t i = 0; i < panel->key_count; i++) {
        if (panel->keys[i].addr.seg == addr.seg && panel->keys[i].addr.key == addr.key) {
            *index = i;
            return true;
        }
    }
    return false;
}

const char *panel_state_name(panel_state_t state)
{
    return state < PANEL_STATE_COUNT ? STATE_NAMES[state] : "?";
}

bool panel_state_from_name(const char *name, panel_state_t *state)
{
    for (int i = 0; i < PANEL_STATE_COUNT; i++) {
        if (strcmp(name, STATE_NAMES[i]) == 0) {
            *state = (panel_state_t)i;
            return true;
        }
    }
    return false;
}
