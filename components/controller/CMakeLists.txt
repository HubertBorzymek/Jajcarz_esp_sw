idf_component_register(SRCS "src/controller.c" "src/boot_test.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES panel presence segments status_led storage
                                  esp_timer esp_app_format freertos)
