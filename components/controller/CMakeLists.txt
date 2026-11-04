idf_component_register(SRCS "src/controller.c" "src/boot_test.c" "src/link.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES net panel presence protocol segments status_led storage sync
                                  esp_timer esp_app_format freertos)
