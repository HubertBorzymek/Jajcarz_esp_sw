// Piny prototypu rev0: ESP32 DevKit V1 na uniwersalce.
#pragma once

#include "driver/gpio.h"

#define BOARD_LED_DATA_GPIO GPIO_NUM_13     // dane WS2812, przez bufor 5 V

// Indeks tablicy to numer klawisza i zarazem pozycja diody w łańcuchu.
static const gpio_num_t BOARD_KEY_GPIOS[] = {
    GPIO_NUM_32,    // SW1
    GPIO_NUM_33,    // SW2
    GPIO_NUM_25,    // SW3
    GPIO_NUM_26,    // SW4
};
#define BOARD_KEY_COUNT (sizeof(BOARD_KEY_GPIOS) / sizeof(BOARD_KEY_GPIOS[0]))
