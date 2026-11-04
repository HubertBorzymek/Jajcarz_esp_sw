idf_component_register(SRCS "src/protocol_parse.c" "src/protocol_encode.c"
                    INCLUDE_DIRS "include"
                    REQUIRES panel jajcarz_types
                    PRIV_REQUIRES espressif__cjson)
