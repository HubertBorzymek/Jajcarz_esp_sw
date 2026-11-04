// Trzyma we flashu stan panelu i synchronizacji, żeby przetrwały restart.
#pragma once

#include "panel.h"
#include "sync.h"

// Wczytuje zapisane dane. Niewysłane zmiany wracają do kolejki i są wysyłane z `base_rev`,
// bo po restarcie czas ich powstania jest nieznany.
void persistence_restore(panel_t *panel, sync_t *sync);

// Zapisuje to, co zmieniło się od poprzedniego wywołania, i loguje zmiany stanów.
void persistence_flush(const panel_t *panel, const sync_t *sync);
