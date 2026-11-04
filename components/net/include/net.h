// Połączenie z serwerem: Wi-Fi i WebSocket. Działa we własnych zadaniach, więc
// wiadomości przychodzące trafiają do kolejki, a użytkownik odbiera je w swoim tempie.
#pragma once

#include <stdbool.h>

#define NET_DEVICE_ID_LEN 13    // 12 znaków hex + '\0'

// Bez konfiguracji Wi-Fi nic nie uruchamia, a panel działa offline.
void net_init(void);

bool net_is_connected(void);

void net_get_device_id(char device_id[NET_DEVICE_ID_LEN]);

bool net_send(const char *text);

// Pobiera wiadomość z kolejki. Odbiorca zwalnia tekst przez net_free().
bool net_receive(char **text);
void net_free(char *text);
