#include "status_led.h"

#include "driver/gpio.h"

#define STATUS_LED_GPIO   GPIO_NUM_2    // dioda na DevKit V1
#define BLINK_PERIOD_MS   1000          // pełny cykl wolnego migania
#define CONFIRM_MS        100           // czas przerwy przy potwierdzeniu

static bool     s_connected;
static bool     s_confirm_active;
static uint32_t s_confirm_start_ms;

esp_err_t status_led_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = 1ULL << STATUS_LED_GPIO,
        .mode         = GPIO_MODE_OUTPUT,
    };
    return gpio_config(&config);
}

void status_led_set_connected(bool connected)
{
    s_connected = connected;
}

void status_led_confirm(uint32_t now_ms)
{
    s_confirm_active = true;
    s_confirm_start_ms = now_ms;
}

void status_led_tick(uint32_t now_ms)
{
    if (s_confirm_active && (now_ms - s_confirm_start_ms) >= CONFIRM_MS) {
        s_confirm_active = false;
    }
    // Potwierdzenie to krótka przerwa, bo po połączeniu dioda świeci stale.
    bool on = s_connected ? !s_confirm_active : (now_ms % BLINK_PERIOD_MS) < (BLINK_PERIOD_MS / 2);
    gpio_set_level(STATUS_LED_GPIO, on);
}
