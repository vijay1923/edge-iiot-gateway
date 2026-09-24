#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "mqtt.h"
#include "ws2812.h"
#include "indicator.h"



static const char *TAG = "MAIN";

static void app_task(void *pvParameters)
{
    indicator_message_t message = {
        .source = IND_SOURCE_SYSTEM,
        .state = IND_STATE_STARTING,
        .severity = IND_SEVERITY_INFO,
        .activity = IND_ACTIVITY_NONE
    };

    indicator_report(message);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void app_main(void)
{
    ESP_ERROR_CHECK(
        nvs_flash_init()
    );

    wifi_init();

    ws2812_init();

    indicator_init();

    xTaskCreate(
        app_task,
        "app_task",
        4096,
        NULL,
        5,
        NULL
    );
}