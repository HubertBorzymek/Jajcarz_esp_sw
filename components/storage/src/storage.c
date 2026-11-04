#include "storage.h"

#include "esp_check.h"
#include "nvs.h"
#include "nvs_flash.h"

#define NAMESPACE "jajcarz"

static const char *const TAG = "storage";

esp_err_t storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "czyszczenie NVS");
        err = nvs_flash_init();
    }
    return err;
}

esp_err_t storage_load(const char *name, void *data, size_t size)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return err;     // przed pierwszym zapisem przestrzeń nazw nie istnieje
    }
    size_t stored_size = size;
    err = nvs_get_blob(handle, name, data, &stored_size);
    nvs_close(handle);
    if (err == ESP_ERR_NVS_INVALID_LENGTH || (err == ESP_OK && stored_size != size)) {
        return ESP_ERR_NVS_NOT_FOUND;   // zapis ze starszej wersji firmware
    }
    return err;
}

esp_err_t storage_save(const char *name, const void *data, size_t size)
{
    nvs_handle_t handle;
    ESP_RETURN_ON_ERROR(nvs_open(NAMESPACE, NVS_READWRITE, &handle), TAG, "otwarcie NVS");
    esp_err_t err = nvs_set_blob(handle, name, data, size);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
