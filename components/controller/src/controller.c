#include "controller.h"

#include <stdint.h>
#include "boot_test.h"
#include "debounce.h"
#include "esp_app_desc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "link.h"
#include "panel.h"
#include "presence.h"
#include "segments.h"
#include "status_led.h"
#include "storage.h"

static const char *const TAG = "controller";

static panel_t    s_panel;
static debounce_t s_debounce[PANEL_MAX_KEYS];

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static size_t collect_addrs(key_addr_t *addrs)
{
    size_t count = 0;
    for (size_t s = 0; s < segments_count(); s++) {
        segment_info_t info = segments_info(s);
        for (uint8_t k = 0; k < info.key_count && count < PANEL_MAX_KEYS; k++) {
            addrs[count++] = (key_addr_t){ .seg = info.id, .key = k };
        }
    }
    return count;
}

static void restore_states(void)
{
    for (size_t i = 0; i < s_panel.key_count; i++) {
        uint8_t state;
        if (storage_load_key_state(s_panel.keys[i].addr, &state) == ESP_OK) {
            panel_set_state(&s_panel, i, (panel_state_t)state);
        }
    }
}

static void init_debounce(void)
{
    uint32_t now = now_ms();
    for (size_t i = 0; i < s_panel.key_count; i++) {
        debounce_init(&s_debounce[i], segments_key_pressed(s_panel.keys[i].addr), now);
    }
}

static void render_leds(presence_mode_t mode, uint32_t now)
{
    static const rgb_t OFF = { 0, 0, 0 };
    bool leds_on = presence_leds_enabled(mode);
    for (size_t i = 0; i < s_panel.key_count; i++) {
        segments_set_led(s_panel.keys[i].addr, leds_on ? panel_led_color(&s_panel, i, now) : OFF);
    }
    ESP_ERROR_CHECK(segments_show());
}

static void on_key_pressed(size_t index, uint32_t now)
{
    const panel_key_t *key = &s_panel.keys[index];
    if (panel_press(&s_panel, index) == PANEL_PRESS_UNASSIGNED) {
        ESP_LOGI(TAG, "%u.%u -> bez produktu", key->addr.seg, key->addr.key);
        link_key_unassigned(index);
        return;
    }
    ESP_LOGI(TAG, "%u.%u -> %s", key->addr.seg, key->addr.key, panel_state_name(key->state));
    esp_err_t err = storage_save_key_state(key->addr, (uint8_t)key->state);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Zapis stanu %u.%u: %s", key->addr.seg, key->addr.key, esp_err_to_name(err));
    }
    link_key_changed(index, now);
}

static void poll_keys(presence_mode_t mode, uint32_t now)
{
    bool keys_on = presence_keys_enabled(mode);
    for (size_t i = 0; i < s_panel.key_count; i++) {
        bool raw = segments_key_pressed(s_panel.keys[i].addr);
        if (debounce_update(&s_debounce[i], raw, now) && keys_on) {
            on_key_pressed(i, now);
        }
    }
}

void controller_init(void)
{
    ESP_LOGI(TAG, "Jajcarz %s", esp_app_get_description()->version);

    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(segments_init());
    ESP_ERROR_CHECK(status_led_init());

    key_addr_t addrs[PANEL_MAX_KEYS];
    size_t count = collect_addrs(addrs);
    panel_init(&s_panel, addrs, count);
    restore_states();

    ESP_ERROR_CHECK(segments_show());   // zgaszenie diod po restarcie
    boot_test_run(addrs, count);
    init_debounce();
    link_init(&s_panel);

    ESP_LOGI(TAG, "Gotowe, klawisze: %u", (unsigned)count);
}

void controller_run(void)
{
    while (1) {
        uint32_t now = now_ms();
        presence_mode_t mode = presence_get_mode();
        poll_keys(mode, now);
        link_tick(now);
        render_leds(mode, now);
        status_led_tick(now);
        // Oddanie procesora na 1 tick, żeby nie zagłodzić zadania IDLE (task watchdog).
        vTaskDelay(1);
    }
}
