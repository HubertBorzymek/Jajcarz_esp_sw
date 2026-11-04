// Klient WebSocket z automatycznym wznawianiem połączenia.
#pragma once

#include <stdbool.h>

void ws_client_init(const char *url, const char *token);

// Wywołanie, gdy klient już działa, nic nie robi.
void ws_client_start(void);

bool ws_client_is_connected(void);
bool ws_client_send(const char *text);
bool ws_client_receive(char **text);
