#include "panel.h"

static const rgb_t COLOR_OFF = { 0, 0, 0 };

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

void panel_init(panel_t *panel, const key_addr_t *addrs, size_t count)
{
    panel->key_count = count < PANEL_MAX_KEYS ? count : PANEL_MAX_KEYS;
    panel->config = DEFAULT_CONFIG;
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

rgb_t panel_led_color(const panel_t *panel, size_t index)
{
    const panel_key_t *key = &panel->keys[index];
    if (!key->assigned) {
        return COLOR_OFF;
    }
    rgb_t color = panel->config.colors[key->state];
    uint8_t brightness = panel->config.brightness;
    return (rgb_t){ scale(color.r, brightness), scale(color.g, brightness), scale(color.b, brightness) };
}

const char *panel_state_name(panel_state_t state)
{
    return state < PANEL_STATE_COUNT ? STATE_NAMES[state] : "?";
}
