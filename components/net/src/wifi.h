// Połączenie Wi-Fi w trybie stacji, z automatycznym ponawianiem.
#pragma once

typedef void (*wifi_ready_cb_t)(void);

// `on_ready` jest wywoływane po każdym uzyskaniu adresu IP.
void wifi_start(const char *ssid, const char *password, wifi_ready_cb_t on_ready);
