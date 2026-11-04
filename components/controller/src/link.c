#include "link.h"

#include "esp_app_desc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "net.h"
#include "protocol.h"
#include "segments.h"
#include "status_led.h"
#include "storage.h"
#include "sync.h"

#define TX_BUFFER_SIZE  256
#define MAX_SEGMENTS    8

static const char *const TAG = "link";

static panel_t       *s_panel;
static sync_t         s_sync;
static protocol_msg_t s_msg;    // statycznie, bo snapshot jest za duży na stos
static bool           s_connected;

static void send_hello(void)
{
    protocol_segment_t segments[MAX_SEGMENTS];
    size_t count = segments_count() < MAX_SEGMENTS ? segments_count() : MAX_SEGMENTS;
    for (size_t i = 0; i < count; i++) {
        segment_info_t info = segments_info(i);
        segments[i] = (protocol_segment_t){ .id = info.id, .key_count = info.key_count };
    }

    char device_id[NET_DEVICE_ID_LEN];
    net_get_device_id(device_id);
    char text[TX_BUFFER_SIZE];
    size_t length = protocol_encode_hello(text, sizeof(text), device_id, esp_app_get_description()->version,
                                          segments, count);
    if (length == 0 || !net_send(text)) {
        ESP_LOGW(TAG, "Nie wysłano hello");
    }
}

static void save_changed_states(const panel_state_t *before)
{
    for (size_t i = 0; i < s_panel->key_count; i++) {
        const panel_key_t *key = &s_panel->keys[i];
        if (key->state == before[i]) {
            continue;
        }
        ESP_LOGI(TAG, "%u.%u -> %s (serwer)", key->addr.seg, key->addr.key, panel_state_name(key->state));
        esp_err_t err = storage_save_key_state(key->addr, (uint8_t)key->state);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Zapis stanu: %s", esp_err_to_name(err));
        }
    }
}

static void apply_identify(const protocol_identify_t *identify, uint32_t now_ms)
{
    size_t index;
    if (panel_find(s_panel, identify->addr, &index)) {
        panel_identify(s_panel, index, identify->duration_ms, now_ms);
    }
}

static void apply_message(const protocol_msg_t *msg, uint32_t now_ms)
{
    panel_state_t before[PANEL_MAX_KEYS];
    for (size_t i = 0; i < s_panel->key_count; i++) {
        before[i] = s_panel->keys[i].state;
    }

    switch (msg->type) {
        case PROTOCOL_MSG_SNAPSHOT:
            sync_apply_snapshot(&s_sync, s_panel, &msg->snapshot);
            break;
        case PROTOCOL_MSG_ACK:
            if (sync_apply_ack(&s_sync, s_panel, &msg->ack)) {
                status_led_confirm(now_ms);
            }
            break;
        case PROTOCOL_MSG_STATE_SET:
            sync_apply_state_set(&s_sync, s_panel, &msg->state_set);
            break;
        case PROTOCOL_MSG_IDENTIFY:
            apply_identify(&msg->identify, now_ms);
            break;
    }
    save_changed_states(before);
}

static void update_connection(void)
{
    bool connected = net_is_connected();
    if (connected == s_connected) {
        return;
    }
    s_connected = connected;
    status_led_set_connected(connected);
    if (connected) {
        send_hello();
    } else {
        sync_on_disconnected(&s_sync);
    }
}

static void receive_messages(uint32_t now_ms)
{
    char *text;
    while (net_receive(&text)) {
        if (protocol_parse(text, &s_msg)) {
            apply_message(&s_msg, now_ms);
        } else {
            ESP_LOGW(TAG, "Nieobsługiwana wiadomość: %s", text);
        }
        net_free(text);
    }
}

static void send_pending_changes(uint32_t now_ms)
{
    sync_outgoing_t out;
    while (sync_next_outgoing(&s_sync, s_panel, now_ms, &out)) {
        const panel_key_t *key = &s_panel->keys[out.index];
        char text[TX_BUFFER_SIZE];
        size_t length = protocol_encode_state_change(text, sizeof(text), out.seq, key->addr, key->state, out.age_ms);
        if (length == 0 || !net_send(text)) {
            ESP_LOGW(TAG, "Nie wysłano zmiany %u.%u", key->addr.seg, key->addr.key);
            return;     // brak ack, więc sync ponowi wysyłkę
        }
    }
}

void link_init(panel_t *panel)
{
    s_panel = panel;
    sync_init(&s_sync);
    net_init();
}

void link_tick(uint32_t now_ms)
{
    update_connection();
    receive_messages(now_ms);
    send_pending_changes(now_ms);
}

void link_key_changed(size_t index, uint32_t now_ms)
{
    sync_local_change(&s_sync, index, now_ms);
}

void link_key_unassigned(size_t index)
{
    char text[TX_BUFFER_SIZE];
    if (protocol_encode_unassigned_press(text, sizeof(text), s_panel->keys[index].addr) != 0) {
        net_send(text);
    }
}
