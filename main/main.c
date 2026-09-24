#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "mqtt.h"
#include "ws2812.h"


static const char *TAG = "MAIN";

static void app_task(void *pvParameters)
{
    int command = 0;

    while (1)
    {
        ESP_LOGI(TAG, "Sending command: %d", command);

        ws2812_send_command(command);

        command++;

        if (command > 2)
        {
            command = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // 
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(
        nvs_flash_init()
    );

    wifi_init();

    ws2812_init();

    xTaskCreate(
        app_task,
        "app_task",
        4096,
        NULL,
        5,
        NULL
    );
}