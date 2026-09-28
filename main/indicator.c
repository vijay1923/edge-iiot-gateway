#include <stdio.h>
#include <stdbool.h>

#include "indicator.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"


static const char *TAG = "INDICATOR";

static QueueHandle_t indicator_queue;  // Queue handle for indicator messages

/* Array to store the latest status of each indicator source */
static indicator_message_t indicator_status[IND_SOURCE_DISPLAY + 1];   // +1 because enum indices start from 0

/* Array to track whether each indicator source has reported a status */
static bool indicator_status_reported[IND_SOURCE_DISPLAY + 1];


/* Function prototypes */

static indicator_visual_t indicator_get_visual(indicator_message_t message);

static indicator_visual_t indicator_get_system_visual(void);

static const char *indicator_color_name(indicator_color_t color);

static const char *indicator_pattern_name(indicator_pattern_t pattern);


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


/* Get the overall severity of all indicator sources */
static indicator_severity_t indicator_get_overall_severity(void)
{
    indicator_severity_t highest_severity = IND_SEVERITY_NONE; // Initialize with the lowest severity

    // Iterate through all indicator sources to find the highest severity
    for (int i = 0; i <= IND_SOURCE_DISPLAY; i++)
    {
        if (indicator_status_reported[i] &&
            indicator_status[i].severity > highest_severity) // Check if the current source has a higher severity than the highest found so far
        {
            highest_severity = indicator_status[i].severity; // Update the highest severity if the current source has a higher severity
        }
    }

    return highest_severity; // Return the highest severity found among all indicator sources
}


/* Indicator Manager Task */
static void indicator_task(void *pvParameters)
{
    indicator_message_t message;

    while (1)
    {
        if (xQueueReceive(indicator_queue, &message, portMAX_DELAY))
        {
            indicator_status[message.source] = message;

            indicator_status_reported[message.source] = true;

            indicator_visual_t visual =
                indicator_get_system_visual();

            ESP_LOGI(
                TAG,
                "Visual: color=%s pattern=%s",
                indicator_color_name(visual.color),
                indicator_pattern_name(visual.pattern)
            );

            indicator_severity_t overall_severity =
                indicator_get_overall_severity();

            ESP_LOGI(
                TAG,
                "Overall severity: %s",
                indicator_severity_name(overall_severity)
            );
        }
    }
}


/* Initialize Indicator Manager */
void indicator_init(void)
{
    indicator_queue = xQueueCreate(10, sizeof(indicator_message_t)); // Create a queue to hold indicator messages

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


/* Get the visual representation of an indicator message */
static indicator_visual_t indicator_get_visual(indicator_message_t message)
{
    indicator_visual_t visual; // Initialize the visual representation structure

    switch (message.state)
    {
        case IND_STATE_CONNECTING:  // Connecting state
            visual.color = IND_COLOR_BLUE;
            visual.pattern = IND_PATTERN_BLINK_SLOW;
            break;

        case IND_STATE_READY:  // Ready state
            visual.color = IND_COLOR_GREEN;
            visual.pattern = IND_PATTERN_SOLID;
            break;

        case IND_STATE_DISCONNECTED:  // Disconnected state
            visual.color = IND_COLOR_YELLOW;
            visual.pattern = IND_PATTERN_BLINK_SLOW;
            break;

        case IND_STATE_WARNING:  // Warning state
            visual.color = IND_COLOR_ORANGE;
            visual.pattern = IND_PATTERN_BLINK_SLOW;
            break;

        case IND_STATE_ERROR:  // Error state
            visual.color = IND_COLOR_RED;
            visual.pattern = IND_PATTERN_SOLID;
            break;

        default:  // Default state
            visual.color = IND_COLOR_OFF;
            visual.pattern = IND_PATTERN_OFF;
            break;
    }

    return visual;
}


/* Get the system visual representation based on all reported indicator sources */
static indicator_visual_t indicator_get_system_visual(void)
{
    indicator_visual_t visual;

    indicator_message_t wifi =
        indicator_status[IND_SOURCE_WIFI];

    indicator_message_t mqtt =
        indicator_status[IND_SOURCE_MQTT];

    bool wifi_reported =
        indicator_status_reported[IND_SOURCE_WIFI];

    bool mqtt_reported =
        indicator_status_reported[IND_SOURCE_MQTT];


    /* Wi-Fi has not reported any status yet */
    if (!wifi_reported)
    {
        visual.color = IND_COLOR_OFF;
        visual.pattern = IND_PATTERN_OFF;

        return visual;
    }


    /* Wi-Fi is connecting */
    if (wifi.state == IND_STATE_CONNECTING)
    {
        visual.color = IND_COLOR_BLUE;
        visual.pattern = IND_PATTERN_BLINK_SLOW;

        return visual;
    }


    /* Wi-Fi is disconnected */
    if (wifi.state == IND_STATE_DISCONNECTED)
    {
        visual.color = IND_COLOR_YELLOW;
        visual.pattern = IND_PATTERN_BLINK_SLOW;

        return visual;
    }


    /* Wi-Fi is ready, now check MQTT */
    if (wifi.state == IND_STATE_READY)
    {
        /* MQTT has not reported any status yet */
        if (!mqtt_reported)
        {
            visual.color = IND_COLOR_PURPLE;
            visual.pattern = IND_PATTERN_BLINK_SLOW;

            return visual;
        }

        /* MQTT is connecting */
        if (mqtt.state == IND_STATE_CONNECTING)
        {
            visual.color = IND_COLOR_PURPLE;
            visual.pattern = IND_PATTERN_BLINK_SLOW;

            return visual;
        }

        /* MQTT is ready */
        if (mqtt.state == IND_STATE_READY)
        {
            visual.color = IND_COLOR_GREEN;
            visual.pattern = IND_PATTERN_SOLID;

            return visual;
        }

        /* MQTT is disconnected */
        if (mqtt.state == IND_STATE_DISCONNECTED)
        {
            visual.color = IND_COLOR_ORANGE;
            visual.pattern = IND_PATTERN_BLINK_SLOW;

            return visual;
        }
    }


    /* Default state */
    visual.color = IND_COLOR_OFF;
    visual.pattern = IND_PATTERN_OFF;

    return visual;
}


/* Get the name of an indicator color */
static const char *indicator_color_name(indicator_color_t color)
{
    switch (color)
    {
        case IND_COLOR_OFF:
            return "OFF";

        case IND_COLOR_BLUE:
            return "BLUE";

        case IND_COLOR_GREEN:
            return "GREEN";

        case IND_COLOR_YELLOW:
            return "YELLOW";

        case IND_COLOR_ORANGE:
            return "ORANGE";

        case IND_COLOR_RED:
            return "RED";

        case IND_COLOR_PURPLE:
            return "PURPLE";

        default:
            return "UNKNOWN";
    }
}


/* Get the name of an indicator pattern */
static const char *indicator_pattern_name(indicator_pattern_t pattern)
{
    switch (pattern)
    {
        case IND_PATTERN_OFF:
            return "OFF";

        case IND_PATTERN_SOLID:
            return "SOLID";

        case IND_PATTERN_BLINK_SLOW:
            return "BLINK_SLOW";

        case IND_PATTERN_BLINK_FAST:
            return "BLINK_FAST";

        default:
            return "UNKNOWN";
    }
}


/* Send a status message to Indicator Manager */
void indicator_report(indicator_message_t message)
{
    xQueueSend(indicator_queue, &message, portMAX_DELAY); // Send the message to the indicator queue
}
