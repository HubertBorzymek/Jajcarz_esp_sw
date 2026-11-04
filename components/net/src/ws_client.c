#include "ws_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define RX_QUEUE_LENGTH     8
#define RX_BUFFER_SIZE      2048
#define PING_INTERVAL_S     30
#define RECONNECT_MS        3000
#define SEND_TIMEOUT_MS     200
#define HEADER_MAX          160
#define OPCODE_CONTINUATION 0x0
#define OPCODE_TEXT         0x1

static const char *const TAG = "ws_client";

static esp_websocket_client_handle_t s_client;
static QueueHandle_t s_rx_queue;
static bool s_started;
static char s_headers[HEADER_MAX];

// Składanie wiadomości z fragmentów. Używane tylko w zadaniu klienta.
static char *s_rx_text;

static void drop_partial_message(void)
{
    free(s_rx_text);
    s_rx_text = NULL;
}

static void on_data(const esp_websocket_event_data_t *data)
{
    if (data->op_code != OPCODE_TEXT && data->op_code != OPCODE_CONTINUATION) {
        return;
    }
    if (data->payload_offset == 0) {
        drop_partial_message();
        s_rx_text = malloc((size_t)data->payload_len + 1);
    }
    if (s_rx_text == NULL) {
        return;
    }
    memcpy(s_rx_text + data->payload_offset, data->data_ptr, (size_t)data->data_len);
    if (data->payload_offset + data->data_len < data->payload_len) {
        return;
    }
    s_rx_text[data->payload_len] = '\0';
    if (xQueueSend(s_rx_queue, &s_rx_text, 0) == pdTRUE) {
        s_rx_text = NULL;   // kolejka przejęła bufor
    } else {
        ESP_LOGW(TAG, "Kolejka pełna, wiadomość pominięta");
        drop_partial_message();
    }
}

static void on_event(void *arg, esp_event_base_t base, int32_t id, void *event_data)
{
    if (id == WEBSOCKET_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "Połączono z serwerem");
    } else if (id == WEBSOCKET_EVENT_DISCONNECTED) {
        ESP_LOGW(TAG, "Rozłączono z serwerem");
        drop_partial_message();
    } else if (id == WEBSOCKET_EVENT_DATA) {
        on_data(event_data);
    }
}

void ws_client_init(const char *url, const char *token)
{
    s_rx_queue = xQueueCreate(RX_QUEUE_LENGTH, sizeof(char *));
    ESP_ERROR_CHECK(s_rx_queue == NULL ? ESP_ERR_NO_MEM : ESP_OK);

    snprintf(s_headers, sizeof(s_headers), "Authorization: Bearer %s\r\n", token);
    esp_websocket_client_config_t config = {
        .uri                  = url,
        .headers              = s_headers,
        .buffer_size          = RX_BUFFER_SIZE,
        .reconnect_timeout_ms = RECONNECT_MS,
        .ping_interval_sec    = PING_INTERVAL_S,
        .crt_bundle_attach    = esp_crt_bundle_attach,
    };
    s_client = esp_websocket_client_init(&config);
    ESP_ERROR_CHECK(s_client == NULL ? ESP_ERR_NO_MEM : ESP_OK);
    ESP_ERROR_CHECK(esp_websocket_register_events(s_client, WEBSOCKET_EVENT_ANY, on_event, NULL));
}

void ws_client_start(void)
{
    if (!s_started) {
        ESP_ERROR_CHECK(esp_websocket_client_start(s_client));
        s_started = true;
    }
}

bool ws_client_is_connected(void)
{
    return s_client != NULL && esp_websocket_client_is_connected(s_client);
}

bool ws_client_send(const char *text)
{
    if (!ws_client_is_connected()) {
        return false;
    }
    int sent = esp_websocket_client_send_text(s_client, text, (int)strlen(text), pdMS_TO_TICKS(SEND_TIMEOUT_MS));
    return sent >= 0;
}

bool ws_client_receive(char **text)
{
    return s_rx_queue != NULL && xQueueReceive(s_rx_queue, text, 0) == pdTRUE;
}
