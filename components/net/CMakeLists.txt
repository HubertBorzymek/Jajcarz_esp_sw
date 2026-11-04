set(config_dir "${CMAKE_CURRENT_LIST_DIR}/../../config")
if(NOT EXISTS "${config_dir}/secrets.h")
    file(COPY_FILE "${config_dir}/secrets.example.h" "${config_dir}/secrets.h")
endif()

idf_component_register(SRCS "src/net.c" "src/wifi.c" "src/ws_client.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "src" "../../config"
                    PRIV_REQUIRES esp_wifi esp_netif esp_event esp_hw_support mbedtls
                                  freertos espressif__esp_websocket_client)
