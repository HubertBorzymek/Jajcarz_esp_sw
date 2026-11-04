// Synchronizacja stanów z serwerem: oczekujące zmiany, ack i reguły rev.
// Czysta logika, bez sieci i RTOS.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "panel.h"
#include "protocol.h"

typedef struct {
    bool     pending;       // zmiana lokalna jeszcze niepotwierdzona
    bool     sent;          // wysłana w tym połączeniu
    uint32_t seq;
    uint32_t changed_ms;
    uint32_t sent_ms;
    uint32_t rev;           // ostatni znany rev klawisza
} sync_key_t;

typedef struct {
    sync_key_t keys[PANEL_MAX_KEYS];
    uint32_t   next_seq;
    bool       ready;       // snapshot odebrany, można wysyłać zmiany
} sync_t;

typedef struct {
    size_t   index;
    uint32_t seq;
    uint32_t age_ms;
} sync_outgoing_t;

void sync_init(sync_t *sync);

// Po utracie połączenia zmiany czekają, aż serwer przyśle kolejny snapshot.
void sync_on_disconnected(sync_t *sync);

void sync_local_change(sync_t *sync, size_t index, uint32_t now_ms);

void sync_apply_snapshot(sync_t *sync, panel_t *panel, const protocol_snapshot_t *snapshot);
void sync_apply_state_set(sync_t *sync, panel_t *panel, const protocol_state_set_t *state_set);

// true, gdy ack dotyczy aktualnej zmiany klawisza.
bool sync_apply_ack(sync_t *sync, panel_t *panel, const protocol_ack_t *ack);

// Następna zmiana do wysłania albo do ponowienia po braku ack.
bool sync_next_outgoing(sync_t *sync, const panel_t *panel, uint32_t now_ms, sync_outgoing_t *out);
