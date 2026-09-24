idf_component_register(
    SRCS
        "main.c"
        "wifi.c"
        "mqtt.c"
        "ws2812.c"
        "indicator.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        nvs_flash
        esp_wifi
        esp_event
        esp_netif
        mqtt
        esp_driver_rmt
        led_strip
)