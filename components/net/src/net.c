#include "net.h"

#include <stdio.h>
#include <stdlib.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "secrets.h"
#include "wifi.h"
#include "ws_client.h"

static const char *const TAG = "net";

void net_get_device_id(char device_id[NET_DEVICE_ID_LEN])
{
    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(mac));
    snprintf(device_id, NET_DEVICE_ID_LEN, "%02x%02x%02x%02x%02x%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void net_init(void)
{
    if (SECRET_WIFI_SSID[0] == '\0') {
        ESP_LOGW(TAG, "Brak konfiguracji Wi-Fi w config/secrets.h, praca offline");
        return;
    }
    ws_client_init(SECRET_SERVER_URL, SECRET_DEVICE_TOKEN);
    wifi_start(SECRET_WIFI_SSID, SECRET_WIFI_PASSWORD, ws_client_start);
}

bool net_is_connected(void)
{
    return ws_client_is_connected();
}

bool net_send(const char *text)
{
    return ws_client_send(text);
}

bool net_receive(char **text)
{
    return ws_client_receive(text);
}

void net_free(char *text)
{
    free(text);
}
