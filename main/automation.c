#include "automation.h"
#include "relay.h"

system_status_t g_system_status = { 0 };
static bool s_auto_mode = true;

/* Tune these for your hardware / wiring */
#define TEMP_THRESHOLD_C      30.0f
#define DARK_ADC_THRESHOLD    2000     // adjust based on LDR divider orientation
#define PRESENCE_THRESHOLD_CM 100.0f

void automation_init(void)
{
    s_auto_mode = true;
}

void automation_set_mode(bool auto_mode)
{
    s_auto_mode = auto_mode;
}

bool automation_get_mode(void)
{
    return s_auto_mode;
}

void automation_run(float temperature, float humidity, int light_level, float distance)
{
    g_system_status.temperature = temperature;
    g_system_status.humidity    = humidity;
    g_system_status.light_level = light_level;
    g_system_status.distance    = distance;
    g_system_status.auto_mode   = s_auto_mode;

    if (s_auto_mode) {
        /* Fan: temperature based */
        relay_set_fan(temperature > TEMP_THRESHOLD_C);

        /* Light: dark AND presence detected */
        bool is_dark  = (light_level > DARK_ADC_THRESHOLD);
        bool presence = (distance > 0.0f && distance < PRESENCE_THRESHOLD_CM);
        relay_set_light(is_dark && presence);
    }
    /* In MANUAL mode, relays are left exactly as the web interface set them. */

    g_system_status.light_state = relay_get_light_state();
    g_system_status.fan_state   = relay_get_fan_state();
}
