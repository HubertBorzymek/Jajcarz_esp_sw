// Łączy panel z serwerem: wysyła zmiany, stosuje wiadomości i zapisuje stany.
#pragma once

#include <stddef.h>
#include <stdint.h>
#include "panel.h"

void link_init(panel_t *panel);

// Wywoływać cyklicznie, nie blokuje na dłużej niż wysyłka jednej wiadomości.
void link_tick(uint32_t now_ms);

void link_key_changed(size_t index, uint32_t now_ms);
void link_key_unassigned(size_t index);
