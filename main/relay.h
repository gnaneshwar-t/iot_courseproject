#ifndef RELAY_H
#define RELAY_H

#include <stdbool.h>

void relay_init(int light_gpio, int fan_gpio);

void relay_set_light(bool on);
void relay_set_fan(bool on);

bool relay_get_light_state(void);
bool relay_get_fan_state(void);

#endif // RELAY_H
