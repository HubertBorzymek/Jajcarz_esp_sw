#include "ws2812.h"

#include "driver/rmt_tx.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"

#define RMT_RESOLUTION_HZ 20000000  // 50 ns/takt, bit = 1,25 us
#define RMT_MEM_SYMBOLS   128
#define BYTES_PER_PIXEL   3

static const char *const TAG = "ws2812";

static rmt_channel_handle_t s_channel;
static rmt_encoder_handle_t s_encoder;

esp_err_t ws2812_init(gpio_num_t gpio)
{
    rmt_tx_channel_config_t channel_config = {
        .gpio_num          = gpio,
        .clk_src           = RMT_CLK_SRC_DEFAULT,
        .resolution_hz     = RMT_RESOLUTION_HZ,
        .mem_block_symbols = RMT_MEM_SYMBOLS,
        .trans_queue_depth = 4,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&channel_config, &s_channel), TAG, "kanał RMT");

    // Czasy wg WS2812B-V5/V6: T0H 0,30 us / T0L 0,95 us, T1H 0,85 us / T1L 0,40 us
    rmt_bytes_encoder_config_t encoder_config = {
        .bit0 = { .level0 = 1, .duration0 = 6,  .level1 = 0, .duration1 = 19 },
        .bit1 = { .level0 = 1, .duration0 = 17, .level1 = 0, .duration1 = 8  },
        .flags.msb_first = 1,
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&encoder_config, &s_encoder), TAG, "enkoder");
    return rmt_enable(s_channel);
}

esp_err_t ws2812_write(const rgb_t *pixels, size_t count)
{
    ESP_RETURN_ON_FALSE(count <= WS2812_MAX_PIXELS, ESP_ERR_INVALID_SIZE, TAG, "za dużo diod");

    uint8_t buffer[WS2812_MAX_PIXELS * BYTES_PER_PIXEL];
    for (size_t i = 0; i < count; i++) {
        buffer[i * BYTES_PER_PIXEL + 0] = pixels[i].g;
        buffer[i * BYTES_PER_PIXEL + 1] = pixels[i].r;
        buffer[i * BYTES_PER_PIXEL + 2] = pixels[i].b;
    }
    rmt_transmit_config_t transmit_config = { .loop_count = 0 };
    ESP_RETURN_ON_ERROR(rmt_transmit(s_channel, s_encoder, buffer, count * BYTES_PER_PIXEL, &transmit_config),
                        TAG, "transmisja");
    return rmt_tx_wait_all_done(s_channel, portMAX_DELAY);
}
