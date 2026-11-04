idf_component_register(SRCS "src/storage.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_common jajcarz_types
                    PRIV_REQUIRES nvs_flash)
