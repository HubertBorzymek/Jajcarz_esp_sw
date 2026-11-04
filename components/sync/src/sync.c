#include "sync.h"

#include <string.h>

#define ACK_TIMEOUT_MS 5000

void sync_init(sync_t *sync)
{
    memset(sync, 0, sizeof(*sync));
    sync->next_seq = 1;
}

void sync_on_disconnected(sync_t *sync)
{
    sync->ready = false;
    for (size_t i = 0; i < PANEL_MAX_KEYS; i++) {
        sync->keys[i].sent = false;
    }
}

void sync_local_change(sync_t *sync, size_t index, uint32_t now_ms)
{
    sync_key_t *key = &sync->keys[index];
    key->pending = true;
    key->sent = false;
    key->seq = sync->next_seq++;
    key->changed_ms = now_ms;
}

static void apply_snapshot_key(sync_t *sync, panel_t *panel, const protocol_key_t *remote)
{
    size_t index;
    if (!panel_find(panel, remote->addr, &index)) {
        return;
    }
    sync_key_t *key = &sync->keys[index];
    panel_set_assigned(panel, index, remote->assigned);
    if (!remote->assigned) {
        key->pending = false;   // klawisz bez produktu nie ma czego zgłaszać
        return;
    }
    key->rev = remote->rev;
    if (!key->pending) {
        panel_set_state(panel, index, remote->state);
    }
}

void sync_apply_snapshot(sync_t *sync, panel_t *panel, const protocol_snapshot_t *snapshot)
{
    for (size_t i = 0; i < snapshot->key_count; i++) {
        apply_snapshot_key(sync, panel, &snapshot->keys[i]);
    }
    sync->ready = true;
}

void sync_apply_state_set(sync_t *sync, panel_t *panel, const protocol_state_set_t *state_set)
{
    size_t index;
    if (!panel_find(panel, state_set->addr, &index)) {
        return;
    }
    sync_key_t *key = &sync->keys[index];
    if (state_set->rev <= key->rev) {
        return;
    }
    key->rev = state_set->rev;
    if (!key->pending) {
        panel_set_state(panel, index, state_set->state);
    }
}

bool sync_apply_ack(sync_t *sync, panel_t *panel, const protocol_ack_t *ack)
{
    size_t index;
    if (!panel_find(panel, ack->addr, &index)) {
        return false;
    }
    sync_key_t *key = &sync->keys[index];
    if (ack->rev > key->rev) {
        key->rev = ack->rev;
    }
    if (!key->pending || ack->seq != key->seq) {
        return false;   // w międzyczasie klawisz naciśnięto ponownie
    }
    key->pending = false;
    panel_set_state(panel, index, ack->state);
    return true;
}

bool sync_next_outgoing(sync_t *sync, const panel_t *panel, uint32_t now_ms, sync_outgoing_t *out)
{
    if (!sync->ready) {
        return false;
    }
    for (size_t i = 0; i < panel->key_count; i++) {
        sync_key_t *key = &sync->keys[i];
        if (!key->pending || (key->sent && (now_ms - key->sent_ms) < ACK_TIMEOUT_MS)) {
            continue;
        }
        key->sent = true;
        key->sent_ms = now_ms;
        *out = (sync_outgoing_t){ .index = i, .seq = key->seq, .age_ms = now_ms - key->changed_ms };
        return true;
    }
    return false;
}
