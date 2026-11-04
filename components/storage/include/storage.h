// Trwały zapis danych panelu we flashu (NVS).
#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "jajcarz_types.h"

esp_err_t storage_init(void);

// ESP_ERR_NVS_NOT_FOUND, gdy dla klawisza nie ma jeszcze zapisu.
esp_err_t storage_load_key_state(key_addr_t addr, uint8_t *state);
esp_err_t storage_save_key_state(key_addr_t addr, uint8_t state);
