// Jajcarz - firmware testowy prototypu panelu przycisków
// ESP32 DevKit V1 (ESP32-WROOM-32), ESP-IDF v6.1
//
// 4 przyciski (do GND, pull-up) + 4 diody WS2812B-V6 (GRB, 800 kHz).
// Każde naciśnięcie przełącza stan przycisku: OK -> LOW -> OUT -> OK ...

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/rmt_tx.h"
#include "esp_timer.h"     // esp_timer_get_time() - jak HAL_GetTick(), ale w mikrosekundach
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ============================================================================
// KONFIGURACJA SPRZĘTU - wszystko w jednym miejscu
// ============================================================================

#define LED_DATA_GPIO     GPIO_NUM_13   // dane WS2812 (przez bufor 5 V)
#define LED_COUNT         4             // liczba diod w łańcuchu

typedef struct {
    const char *name;       // nazwa w logach
    gpio_num_t  gpio;       // pin przycisku
    uint8_t     led_index;  // indeks diody w łańcuchu (0 = pierwsza)
} button_cfg_t;

static const button_cfg_t BUTTONS[] = {
    { "SW1", GPIO_NUM_32, 0 },
    { "SW2", GPIO_NUM_33, 1 },
    { "SW3", GPIO_NUM_25, 2 },
    { "SW4", GPIO_NUM_26, 3 },
};
#define BUTTON_COUNT (sizeof(BUTTONS) / sizeof(BUTTONS[0]))

#define DEBOUNCE_MS       30     // czas stabilizacji styku
#define BOOT_TEST_STEP_MS 300    // czas jednego koloru w teście startowym

// ============================================================================
// KOLORY I JASNOŚĆ - stałe do łatwej korekty
// ============================================================================

typedef struct {
    uint8_t r, g, b;
} rgb_t;

static const rgb_t COLOR_OK   = { 0,   255, 0 };   // zielony
static const rgb_t COLOR_LOW  = { 255, 120, 0 };   // żółty (mniej G, żeby nie wpadał w zieleń)
static const rgb_t COLOR_OUT  = { 255, 0,   0 };   // czerwony
static const rgb_t COLOR_OFF  = { 0,   0,   0 };

#define LED_BRIGHTNESS    51     // globalna jasność 0..255 (51 ≈ 20%)

// ============================================================================
// STANY PRZYCISKÓW
// ============================================================================

typedef enum {
    STATE_OK = 0,
    STATE_LOW,
    STATE_OUT,
    STATE_COUNT
} button_state_t;

static const char *const STATE_NAMES[STATE_COUNT] = { "OK", "LOW", "OUT" };

// Dane debouncingu dla jednego przycisku
typedef struct {
    bool     raw;           // ostatni surowy odczyt (true = naciśnięty)
    bool     stable;        // stan po debouncingu
    uint32_t raw_change_ms; // kiedy surowy odczyt ostatnio się zmienił
} debounce_t;

static debounce_t     s_debounce[BUTTON_COUNT];
static button_state_t s_state[BUTTON_COUNT];
static rgb_t          s_leds[LED_COUNT];
static bool           s_leds_dirty;

// ============================================================================
// CZAS
// ============================================================================

static inline uint32_t millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// ============================================================================
// ŹRÓDŁO ODCZYTU PRZYCISKÓW
// Tu podmienia się sprzęt (np. ekspander I²C) - reszta kodu widzi tylko
// funkcję buttons_read_raw(), zwracającą "czy naciśnięty" dla indeksu.
// ============================================================================

static void buttons_hw_init(void)
{
    uint64_t mask = 0;
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        mask |= 1ULL << BUTTONS[i].gpio;
    }
    gpio_config_t cfg = {
        .pin_bit_mask = mask,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));
}

// true = przycisk naciśnięty (stan niski na pinie)
static bool buttons_read_raw(size_t i)
{
    return gpio_get_level(BUTTONS[i].gpio) == 0;
}

// ============================================================================
// DEBOUNCING - nieblokujący, osobny dla każdego przycisku
// Zwraca true tylko w momencie naciśnięcia (zbocze opadające po debouncingu).
// ============================================================================

static bool debounce_update(debounce_t *d, bool raw_now, uint32_t now_ms)
{
    if (raw_now != d->raw) {
        d->raw = raw_now;
        d->raw_change_ms = now_ms;
        return false;
    }
    if (d->stable != d->raw && (now_ms - d->raw_change_ms) >= DEBOUNCE_MS) {
        d->stable = d->raw;
        return d->stable;   // true tylko przy przejściu na "naciśnięty"
    }
    return false;
}

// ============================================================================
// LOGIKA STANÓW
// ============================================================================

static rgb_t state_to_color(button_state_t st)
{
    switch (st) {
        case STATE_OK:  return COLOR_OK;
        case STATE_LOW: return COLOR_LOW;
        case STATE_OUT: return COLOR_OUT;
        default:        return COLOR_OFF;
    }
}

static void led_set(uint8_t index, rgb_t color)
{
    if (index >= LED_COUNT) {
        return;
    }
    if (s_leds[index].r != color.r || s_leds[index].g != color.g || s_leds[index].b != color.b) {
        s_leds[index] = color;
        s_leds_dirty = true;
    }
}

static void led_set_all(rgb_t color)
{
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        led_set(i, color);
    }
}

// Ustawia stan przycisku i odwzorowuje go na diodę
static void button_set_state(size_t i, button_state_t st)
{
    s_state[i] = st;
    led_set(BUTTONS[i].led_index, state_to_color(st));
}

// Wywoływane przy każdym naciśnięciu - miejsce na przyszłe powiadomienia (np. sieć)
static void on_button_pressed(size_t i)
{
    button_state_t next = (button_state_t)((s_state[i] + 1) % STATE_COUNT);
    button_set_state(i, next);
    printf("%s -> %s\n", BUTTONS[i].name, STATE_NAMES[next]);
}

static void buttons_poll(void)
{
    uint32_t now = millis();
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        if (debounce_update(&s_debounce[i], buttons_read_raw(i), now)) {
            on_button_pressed(i);
        }
    }
}

// ============================================================================
// STEROWNIK WS2812 (RMT)
// Rozdzielczość 20 MHz (50 ns/takt), bit = 1,25 us -> 800 kHz.
// ============================================================================

#define RMT_RESOLUTION_HZ 20000000

static rmt_channel_handle_t s_rmt_chan;
static rmt_encoder_handle_t s_rmt_encoder;

static void leds_hw_init(void)
{
    rmt_tx_channel_config_t chan_cfg = {
        .gpio_num          = LED_DATA_GPIO,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = RMT_RESOLUTION_HZ,
        .mem_block_symbols = 64,
        .trans_queue_depth = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_cfg, &s_rmt_chan));

    // Czasy wg WS2812B-V5/V6: T0H 0,30 us / T0L 0,95 us, T1H 0,85 us / T1L 0,40 us
    rmt_bytes_encoder_config_t enc_cfg = {
        .bit0 = { .level0 = 1, .duration0 = 6,  .level1 = 0, .duration1 = 19 },
        .bit1 = { .level0 = 1, .duration0 = 17, .level1 = 0, .duration1 = 8  },
        .flags.msb_first = 1,
    };
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&enc_cfg, &s_rmt_encoder));
    ESP_ERROR_CHECK(rmt_enable(s_rmt_chan));
}

static inline uint8_t scale(uint8_t v)
{
    return (uint8_t)(((uint16_t)v * LED_BRIGHTNESS) / 255);
}

// Wysyła bufor do diod tylko gdy coś się zmieniło.
// Po transmisji linia zostaje w stanie niskim - to jest reset/latch (>280 us),
// a kolejna transmisja i tak nastąpi najwcześniej w następnej iteracji pętli.
static void leds_update(void)
{
    if (!s_leds_dirty) {
        return;
    }
    uint8_t buf[LED_COUNT * 3];
    for (size_t i = 0; i < LED_COUNT; i++) {
        buf[i * 3 + 0] = scale(s_leds[i].g);   // kolejność GRB
        buf[i * 3 + 1] = scale(s_leds[i].r);
        buf[i * 3 + 2] = scale(s_leds[i].b);
    }
    rmt_transmit_config_t tx_cfg = { .loop_count = 0 };
    ESP_ERROR_CHECK(rmt_transmit(s_rmt_chan, s_rmt_encoder, buf, sizeof(buf), &tx_cfg));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(s_rmt_chan, portMAX_DELAY));
    s_leds_dirty = false;
}

// ============================================================================
// TEST STARTOWY - jedyne miejsce z opóźnieniem blokującym
// ============================================================================

static void boot_test(void)
{
    static const rgb_t seq[] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 } };
    for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); i++) {
        led_set_all(seq[i]);
        leds_update();
        vTaskDelay(pdMS_TO_TICKS(BOOT_TEST_STEP_MS));
    }
}

// ============================================================================

void app_main(void)
{
    printf("\nJajcarz - panel przyciskow, firmware testowy\n");

    buttons_hw_init();
    leds_hw_init();

    s_leds_dirty = true;   // wymuś pierwszą wysyłkę (zgaszenie diod)
    leds_update();
    boot_test();

    // Stan początkowy: wszystkie OK. Debouncing startuje od bieżącego odczytu,
    // żeby przycisk wciśnięty przy starcie nie dał zmiany stanu.
    uint32_t now = millis();
    for (size_t i = 0; i < BUTTON_COUNT; i++) {
        bool raw = buttons_read_raw(i);
        s_debounce[i] = (debounce_t){ .raw = raw, .stable = raw, .raw_change_ms = now };
        button_set_state(i, STATE_OK);
    }
    leds_update();
    printf("Gotowe, wszystkie przyciski: OK\n");

    // Pętla główna - nieblokująca, sprawdza czas
    while (1) {
        buttons_poll();
        leds_update();
        // Oddanie procesora na 1 tick (10 ms przy CONFIG_FREERTOS_HZ=100),
        // żeby nie zagłodzić zadania IDLE (task watchdog). To nie jest
        // opóźnienie logiki - debouncing liczy się od millis().
        vTaskDelay(1);
    }
}
