idf_component_register(SRCS "src/segments.c" "src/segment_local.c" "src/ws2812.c"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "src"
                    REQUIRES jajcarz_types esp_common
                    PRIV_REQUIRES esp_driver_gpio esp_driver_rmt)
