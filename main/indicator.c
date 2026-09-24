#include "indicator.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "INDICATOR";

static QueueHandle_t indicator_queue;  // Queue handle for indicator messages
/* Array to store the latest status of each indicator source */
static indicator_message_t indicator_status[IND_SOURCE_DISPLAY + 1];   // +1 because enum indices start from 0

/* Get the name of the indicator source for logging purposes */
static const char *indicator_source_name(indicator_source_t source)
{
    switch (source)
    {
        case IND_SOURCE_SYSTEM:
            return "SYSTEM";

        case IND_SOURCE_WIFI:
            return "WIFI";

        case IND_SOURCE_MQTT:
            return "MQTT";

        case IND_SOURCE_RS485:
            return "RS485";

        case IND_SOURCE_ETHERNET:
            return "ETHERNET";

        case IND_SOURCE_USB:
            return "USB";

        case IND_SOURCE_IO:
            return "IO";

        case IND_SOURCE_DISPLAY:
            return "DISPLAY";

        default:
            return "UNKNOWN";
    }
}
/* Get the name of the indicator state for logging purposes */
static const char *indicator_state_name(indicator_state_t state)
{
    switch (state)
    {
        case IND_STATE_STARTING:
            return "STARTING";

        case IND_STATE_CONNECTING:
            return "CONNECTING";

        case IND_STATE_READY:
            return "READY";

        case IND_STATE_DISCONNECTED:
            return "DISCONNECTED";

        case IND_STATE_WARNING:
            return "WARNING";

        case IND_STATE_ERROR:
            return "ERROR";

        default:
            return "UNKNOWN";
    }
}

/* Get the name of the indicator severity for logging purposes */
static const char *indicator_severity_name(indicator_severity_t severity)
{
    switch (severity)
    {
        case IND_SEVERITY_NONE:
            return "NONE";

        case IND_SEVERITY_INFO:
            return "INFO";

        case IND_SEVERITY_WARNING:
            return "WARNING";

        case IND_SEVERITY_ERROR:
            return "ERROR";

        case IND_SEVERITY_CRITICAL:
            return "CRITICAL";

        default:
            return "UNKNOWN";
    }
}

/* Get the name of the indicator activity for logging purposes */
static const char *indicator_activity_name(indicator_activity_t activity)
{
    switch (activity)
    {
        case IND_ACTIVITY_NONE:
            return "NONE";

        case IND_ACTIVITY_RX:
            return "RX";

        case IND_ACTIVITY_TX:
            return "TX";

        case IND_ACTIVITY_EVENT:
            return "EVENT";

        default:
            return "UNKNOWN";
    }
}


/* Indicator Manager Task */
static void indicator_task(void *pvParameters)
{
    indicator_message_t message;

    while (1)
    {
        if (xQueueReceive( indicator_queue,&message,portMAX_DELAY)) // Wait for a message from the indicator queue
        {
            // Update the latest status for the source
            indicator_status[message.source] = message;

            ESP_LOGI(
                TAG,
                "Message received: source=%s state=%s severity=%s activity=%s",
                indicator_source_name(message.source),
                indicator_state_name(message.state),
                indicator_severity_name(message.severity),
                indicator_activity_name(message.activity)
            );
        }
    }
}

/* Initialize Indicator Manager */
void indicator_init(void)
{
    indicator_queue = xQueueCreate(10,sizeof(indicator_message_t)); // Create a queue to hold indicator messages

    if (indicator_queue == NULL)
    {
        ESP_LOGE(TAG, "Failed to create indicator queue");
        return;
    }

    /* Creating Indicator Manager Task */
    xTaskCreate(   
        indicator_task,
        "indicator_task",
        4096,
        NULL,
        5,
        NULL
    );

    ESP_LOGI(TAG, "Indicator Manager initialized");
}

/* Send a status message to Indicator Manager */
void indicator_report(indicator_message_t message)
{
    xQueueSend(indicator_queue,&message,portMAX_DELAY); // Send the message to the indicator queue
}