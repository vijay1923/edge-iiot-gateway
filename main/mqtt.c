#include "mqtt.h"

#include "mqtt_client.h"
#include "esp_log.h"

#include "config.h"

#define MQTT_TOPIC "test/esp32"

static const char *TAG = "MQTT";


/* MQTT Event Handler */
static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event_id)
    {
        case MQTT_EVENT_CONNECTED:

            ESP_LOGI(TAG, "MQTT connected");

            esp_mqtt_client_publish(
                event->client,
                MQTT_TOPIC,
                "Hello from ESP32-S3",
                0,
                1,
                0
            );

            ESP_LOGI(TAG, "Message published");

            break;


        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGI(TAG, "MQTT disconnected");

            break;


        default:

            break;
    }
}


/* Start MQTT */
void mqtt_start(void)
{
    ESP_LOGI(TAG, "Starting MQTT...");

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_SERVER,
        .credentials.username = MQTT_USER,
        .credentials.authentication.password = MQTT_PASS,
    };

    esp_mqtt_client_handle_t client =
        esp_mqtt_client_init(&mqtt_cfg);

    ESP_ERROR_CHECK(
        esp_mqtt_client_register_event(
            client,
            ESP_EVENT_ANY_ID,
            mqtt_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_mqtt_client_start(client)
    );
    ESP_LOGI(TAG, "MQTT client started");
}