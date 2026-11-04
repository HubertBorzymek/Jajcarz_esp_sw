#include "persistence.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "storage.h"

#define CONFIG_NAME "config"

typedef struct {
    uint32_t rev;
    uint8_t  state;
    uint8_t  assigned;
    uint8_t  pending;
    uint8_t  reserved;
} key_record_t;

static const char *const TAG = "persistence";

static key_record_t   s_saved_keys[PANEL_MAX_KEYS];
static panel_config_t s_saved_config;

// Nazwa w NVS, np. "k0.2". Najdłuższa ma 8 znaków.
static void record_name(key_addr_t addr, char name[STORAGE_NAME_MAX + 1])
{
    snprintf(name, STORAGE_NAME_MAX + 1, "k%u.%u", addr.seg, addr.key);
}

static key_record_t make_record(const panel_key_t *key, const sync_key_t *sync_key)
{
    return (key_record_t){
        .rev      = sync_key->rev,
        .state    = (uint8_t)key->state,
        .assigned = key->assigned,
        .pending  = sync_key->pending,
    };
}

static bool same_config(const panel_config_t *a, const panel_config_t *b)
{
    if (a->brightness != b->brightness) {
        return false;
    }
    for (int i = 0; i < PANEL_STATE_COUNT; i++) {
        const rgb_t *x = &a->colors[i];
        const rgb_t *y = &b->colors[i];
        if (x->r != y->r || x->g != y->g || x->b != y->b) {
            return false;
        }
    }
    return true;
}

static void report_error(const char *what, esp_err_t err)
{
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Zapis %s: %s", what, esp_err_to_name(err));
    }
}

void persistence_restore(panel_t *panel, sync_t *sync)
{
    for (size_t i = 0; i < panel->key_count; i++) {
        const panel_key_t *key = &panel->keys[i];
        char name[STORAGE_NAME_MAX + 1];
        record_name(key->addr, name);

        key_record_t record;
        if (storage_load(name, &record, sizeof(record)) == ESP_OK) {
            panel_set_state(panel, i, (panel_state_t)record.state);
            panel_set_assigned(panel, i, record.assigned != 0);
            sync_restore_key(sync, i, record.pending != 0, record.rev);
        }
        s_saved_keys[i] = make_record(key, &sync->keys[i]);
    }

    panel_config_t config;
    if (storage_load(CONFIG_NAME, &config, sizeof(config)) == ESP_OK) {
        panel->config = config;
    }
    s_saved_config = panel->config;
}

void persistence_flush(const panel_t *panel, const sync_t *sync)
{
    for (size_t i = 0; i < panel->key_count; i++) {
        const panel_key_t *key = &panel->keys[i];
        key_record_t record = make_record(key, &sync->keys[i]);
        key_record_t *saved = &s_saved_keys[i];
        if (record.rev == saved->rev && record.state == saved->state
            && record.assigned == saved->assigned && record.pending == saved->pending) {
            continue;
        }
        if (record.state != saved->state) {
            ESP_LOGI(TAG, "%u.%u -> %s", key->addr.seg, key->addr.key, panel_state_name(key->state));
        }
        char name[STORAGE_NAME_MAX + 1];
        record_name(key->addr, name);
        report_error(name, storage_save(name, &record, sizeof(record)));
        *saved = record;
    }

    if (!same_config(&panel->config, &s_saved_config)) {
        ESP_LOGI(TAG, "Nowa konfiguracja, jasność %u", panel->config.brightness);
        report_error(CONFIG_NAME, storage_save(CONFIG_NAME, &panel->config, sizeof(panel->config)));
        s_saved_config = panel->config;
    }
}
