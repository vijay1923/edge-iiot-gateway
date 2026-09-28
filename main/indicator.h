#ifndef INDICATOR_H
#define INDICATOR_H

//  indicator sources
typedef enum
{
    IND_SOURCE_SYSTEM, // System source
    IND_SOURCE_WIFI, // WiFi source
    IND_SOURCE_MQTT, // MQTT source
    IND_SOURCE_RS485, // RS485 source
    IND_SOURCE_ETHERNET, // Ethernet source
    IND_SOURCE_USB, // USB source
    IND_SOURCE_IO, // IO source
    IND_SOURCE_DISPLAY // Display source
} indicator_source_t;

// indicator states
typedef enum
{
    IND_STATE_STARTING, // Starting state
    IND_STATE_CONNECTING, // Connecting state
    IND_STATE_READY, // Ready state
    IND_STATE_DISCONNECTED, // Disconnected state
    IND_STATE_WARNING, // Warning state
    IND_STATE_ERROR // Error state  
} indicator_state_t;

//  indicator severities
typedef enum
{
    IND_SEVERITY_NONE, // No severity
    IND_SEVERITY_INFO, // Informational severity
    IND_SEVERITY_WARNING, // Warning severity
    IND_SEVERITY_ERROR, // Error severity
    IND_SEVERITY_CRITICAL // Critical severity
} indicator_severity_t;

//  indicator activities 
typedef enum
{
    IND_ACTIVITY_NONE, // No activity
    IND_ACTIVITY_RX, // Receive activity
    IND_ACTIVITY_TX, // Transmit activity
    IND_ACTIVITY_EVENT // Event activity    
} indicator_activity_t;

// struct for indicator message containing source, state, severity, and activity
typedef struct
{
    indicator_source_t source; // Source of the indicator
    indicator_state_t state; // State of the indicator
    indicator_severity_t severity; // Severity of the indicator
    indicator_activity_t activity; // Activity of the indicator

} indicator_message_t;

/* Indicator colors and patterns */
typedef enum
{
    IND_COLOR_OFF,
    IND_COLOR_BLUE,
    IND_COLOR_GREEN,
    IND_COLOR_YELLOW,
    IND_COLOR_ORANGE,
    IND_COLOR_RED,
    IND_COLOR_PURPLE
} indicator_color_t;

/* Indicator patterns */
typedef enum
{
    IND_PATTERN_OFF,
    IND_PATTERN_SOLID,
    IND_PATTERN_BLINK_SLOW,
    IND_PATTERN_BLINK_FAST
} indicator_pattern_t;

/* Indicator visual representation */
typedef struct
{
    indicator_color_t color;
    indicator_pattern_t pattern;
} indicator_visual_t;

void indicator_init(void);
void indicator_report(indicator_message_t message);

#endif