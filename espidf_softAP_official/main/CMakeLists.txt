idf_component_register(
    SRCS "softap_example_main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES
        esp_wifi
        esp_http_server
        esp_driver_gpio
        esp_netif
        esp_event
        nvs_flash
)
