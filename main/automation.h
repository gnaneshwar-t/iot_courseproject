#ifndef AUTOMATION_H
#define AUTOMATION_H

#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    int   light_level;   // raw ADC 0-4095
    float distance;      // cm, -1 if invalid
    bool  light_state;
    bool  fan_state;
    bool  auto_mode;
} system_status_t;

/* Latest sensor/actuator snapshot, read by the HTTP server for /status. */
extern system_status_t g_system_status;

void automation_init(void);

/* AUTO mode: decide relay states from sensor readings.
 * MANUAL mode: only refreshes g_system_status, relays are left as the
 * user set them via the web interface. */
void automation_run(float temperature, float humidity, int light_level, float distance);

void automation_set_mode(bool auto_mode);
bool automation_get_mode(void);

#endif // AUTOMATION_H
