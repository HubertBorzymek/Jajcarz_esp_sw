#include "boot_test.h"

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "segments.h"

#define STEP_MS 300

static const rgb_t SEQUENCE[] = { { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 } };

void boot_test_run(const key_addr_t *addrs, size_t count)
{
    for (size_t step = 0; step < sizeof(SEQUENCE) / sizeof(SEQUENCE[0]); step++) {
        for (size_t i = 0; i < count; i++) {
            segments_set_led(addrs[i], SEQUENCE[step]);
        }
        ESP_ERROR_CHECK(segments_show());
        vTaskDelay(pdMS_TO_TICKS(STEP_MS));
    }
}
