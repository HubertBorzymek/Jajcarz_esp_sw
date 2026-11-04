// Wzór lokalnej konfiguracji. Build kopiuje go do config/secrets.h, a tamten plik
// jest poza gitem. Puste SECRET_WIFI_SSID oznacza pracę bez sieci.
#pragma once

#define SECRET_WIFI_SSID     ""
#define SECRET_WIFI_PASSWORD ""
#define SECRET_SERVER_URL    "ws://192.168.0.2:5080/ws/device"
#define SECRET_DEVICE_TOKEN  "test-token"
