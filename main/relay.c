#include "relay.h"
#include "driver/gpio.h"

static int  s_light_gpio;
static int  s_fan_gpio;
static bool s_light_state = false;
static bool s_fan_state   = false;

void relay_init(int light_gpio, int fan_gpio)
{
    s_light_gpio = light_gpio;
    s_fan_gpio   = fan_gpio;

    gpio_reset_pin(s_light_gpio);
    gpio_reset_pin(s_fan_gpio);
    gpio_set_direction(s_light_gpio, GPIO_MODE_OUTPUT);
    gpio_set_direction(s_fan_gpio, GPIO_MODE_OUTPUT);

    gpio_set_level(s_light_gpio, 0);
    gpio_set_level(s_fan_gpio, 0);
    s_light_state = false;
    s_fan_state   = false;
}

void relay_set_light(bool on)
{
    s_light_state = on;
    gpio_set_level(s_light_gpio, on ? 1 : 0);
}

void relay_set_fan(bool on)
{
    s_fan_state = on;
    gpio_set_level(s_fan_gpio, on ? 1 : 0);
}

bool relay_get_light_state(void) { return s_light_state; }
bool relay_get_fan_state(void)   { return s_fan_state; }
