// Sygnalizator stanu urządzenia. W rev0 dioda na płytce DevKit.
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t status_led_init(void);

// Połączono: świeci stale. Brak połączenia: wolno miga.
void status_led_set_connected(bool connected);

// Krótkie mignięcie: serwer potwierdził zmianę.
void status_led_confirm(uint32_t now_ms);

// Wywoływać cyklicznie, nie blokuje.
void status_led_tick(uint32_t now_ms);
