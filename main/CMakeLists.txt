idf_component_register(SRCS "softap_sta.c"
                    PRIV_REQUIRES esp_wifi nvs_flash esp_http_server esp_timer
                    REQUIRES esp_netif esp_event
                    INCLUDE_DIRS ".")
