// Segment 0: klawisze na GPIO (do GND, pull-up) i łańcuch WS2812 na płytce głównej.
#include <stdbool.h>
#include "esp_check.h"
#include "board_rev0.h"
#include "segment_driver.h"
#include "ws2812.h"

static const char *const TAG = "segment_local";

static rgb_t s_leds[BOARD_KEY_COUNT];
static bool  s_leds_dirty = true;   // pierwsza wysyłka gasi diody po restarcie

static esp_err_t local_init(void)
{
    uint64_t mask = 0;
    for (size_t i = 0; i < BOARD_KEY_COUNT; i++) {
        mask |= 1ULL << BOARD_KEY_GPIOS[i];
    }
    gpio_config_t config = {
        .pin_bit_mask = mask,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&config), TAG, "klawisze");
    return ws2812_init(BOARD_LED_DATA_GPIO);
}

static bool local_key_pressed(uint8_t key)
{
    return key < BOARD_KEY_COUNT && gpio_get_level(BOARD_KEY_GPIOS[key]) == 0;
}

static void local_set_led(uint8_t key, rgb_t color)
{
    if (key >= BOARD_KEY_COUNT) {
        return;
    }
    rgb_t *led = &s_leds[key];
    if (led->r != color.r || led->g != color.g || led->b != color.b) {
        *led = color;
        s_leds_dirty = true;
    }
}

static esp_err_t local_show(void)
{
    if (!s_leds_dirty) {
        return ESP_OK;
    }
    ESP_RETURN_ON_ERROR(ws2812_write(s_leds, BOARD_KEY_COUNT), TAG, "diody");
    s_leds_dirty = false;
    return ESP_OK;
}

const segment_driver_t segment_local_driver = {
    .id          = 0,
    .key_count   = BOARD_KEY_COUNT,
    .init        = local_init,
    .key_pressed = local_key_pressed,
    .set_led     = local_set_led,
    .show        = local_show,
};
