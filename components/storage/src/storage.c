#include "storage.h"

#include <stdio.h>
#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"

#define NAMESPACE "jajcarz"

static const char *const TAG = "storage";

// Klucz NVS, np. "key0.2". Limit to 15 znaków, a najdłuższy adres ma 10.
static void key_name(key_addr_t addr, char name[NVS_KEY_NAME_MAX_SIZE])
{
    snprintf(name, NVS_KEY_NAME_MAX_SIZE, "key%u.%u", addr.seg, addr.key);
}

esp_err_t storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "czyszczenie NVS");
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t storage_load_key_state(key_addr_t addr, uint8_t *state)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;     // przed pierwszym zapisem przestrzeń nazw nie istnieje
    }
    char name[NVS_KEY_NAME_MAX_SIZE];
    key_name(addr, name);
    err = nvs_get_u8(handle, name, state);
    nvs_close(handle);
    return err;
}

esp_err_t storage_save_key_state(key_addr_t addr, uint8_t state)
{
    nvs_handle_t handle;
    ESP_RETURN_ON_ERROR(nvs_open(NAMESPACE, NVS_READWRITE, &handle), TAG, "otwarcie NVS");
    char name[NVS_KEY_NAME_MAX_SIZE];
    key_name(addr, name);
    esp_err_t err = nvs_set_u8(handle, name, state);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
