// Trwały zapis bloków danych we flashu (NVS).
#pragma once

#include <stddef.h>
#include "esp_err.h"

#define STORAGE_NAME_MAX 15     // limit długości nazwy w NVS

esp_err_t storage_init(void);

// ESP_ERR_NVS_NOT_FOUND, gdy nie ma zapisu albo ma on inny rozmiar niż `size`.
esp_err_t storage_load(const char *name, void *data, size_t size);
esp_err_t storage_save(const char *name, const void *data, size_t size);
