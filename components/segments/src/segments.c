#include "segments.h"

#include "esp_check.h"
#include "segment_driver.h"

static const char *const TAG = "segments";

static const segment_driver_t *const DRIVERS[] = {
    &segment_local_driver,
};
#define DRIVER_COUNT (sizeof(DRIVERS) / sizeof(DRIVERS[0]))

static const segment_driver_t *find_driver(uint8_t id)
{
    for (size_t i = 0; i < DRIVER_COUNT; i++) {
        if (DRIVERS[i]->id == id) {
            return DRIVERS[i];
        }
    }
    return NULL;
}

esp_err_t segments_init(void)
{
    for (size_t i = 0; i < DRIVER_COUNT; i++) {
        ESP_RETURN_ON_ERROR(DRIVERS[i]->init(), TAG, "segment %u", DRIVERS[i]->id);
    }
    return ESP_OK;
}

size_t segments_count(void)
{
    return DRIVER_COUNT;
}

segment_info_t segments_info(size_t index)
{
    return (segment_info_t){ .id = DRIVERS[index]->id, .key_count = DRIVERS[index]->key_count };
}

bool segments_key_pressed(key_addr_t addr)
{
    const segment_driver_t *driver = find_driver(addr.seg);
    return driver != NULL && driver->key_pressed(addr.key);
}

void segments_set_led(key_addr_t addr, rgb_t color)
{
    const segment_driver_t *driver = find_driver(addr.seg);
    if (driver != NULL) {
        driver->set_led(addr.key, color);
    }
}

esp_err_t segments_show(void)
{
    for (size_t i = 0; i < DRIVER_COUNT; i++) {
        ESP_RETURN_ON_ERROR(DRIVERS[i]->show(), TAG, "segment %u", DRIVERS[i]->id);
    }
    return ESP_OK;
}
