#include "ws2812.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "esp_log.h"

#define WS2812_GPIO 15

static const char *TAG = "WS2812";

static led_strip_handle_t led_strip;
static QueueHandle_t ws2812_queue;

static void ws2812_task(void *pvParameters)
{
    int command;

    while (1)
    {
        if (xQueueReceive(ws2812_queue,&command,portMAX_DELAY)) 
        {
            switch (command)
            {
                case 0:

                    ESP_LOGI(TAG, "RED");

                    led_strip_set_pixel( 
                        led_strip, // LED strip handle
                        0, // LED index
                        255, // Red
                        0,   // Green
                        0    // Blue
                    );
                    led_strip_refresh(led_strip);

                    break;

                case 1:

                    ESP_LOGI(TAG, "GREEN");

                    led_strip_set_pixel(
                        led_strip, // LED strip handle
                        0,         // LED index
                        0,   // Red
                        255, // Green
                        0    // Blue
                    );

                    led_strip_refresh(led_strip);

                    break;

                case 2:

                    ESP_LOGI(TAG, "BLUE");

                    led_strip_set_pixel(
                        led_strip, // LED strip handle
                        0,    // LED index
                        0,   // Red
                        0,   // Green
                        255  // Blue
                    );

                    led_strip_refresh(led_strip);

                    break;
            }
        }
    }
}

void ws2812_init(void)
{
    ESP_LOGI(TAG, "Initializing WS2812");

    led_strip_config_t strip_config = {
        .strip_gpio_num = WS2812_GPIO,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ws2812_queue = xQueueCreate(
        5,
        sizeof(int)
    );

    xTaskCreate(
        ws2812_task,
        "ws2812_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "WS2812 initialized");
}

void ws2812_send_command(int command) 
{
    xQueueSend(ws2812_queue,&command,portMAX_DELAY); // Send command to WS2812 task
}