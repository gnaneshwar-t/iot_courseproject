#include "hcsr04.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_timer.h"

#define ECHO_TIMEOUT_US 30000  // ~5m round trip max

static int s_trig_gpio;
static int s_echo_gpio;

void hcsr04_init(int trig_gpio, int echo_gpio)
{
    s_trig_gpio = trig_gpio;
    s_echo_gpio = echo_gpio;

    gpio_reset_pin(s_trig_gpio);
    gpio_reset_pin(s_echo_gpio);
    gpio_set_direction(s_trig_gpio, GPIO_MODE_OUTPUT);
    gpio_set_direction(s_echo_gpio, GPIO_MODE_INPUT);
    gpio_set_level(s_trig_gpio, 0);
}

float hcsr04_get_distance_cm(void)
{
    /* 10us trigger pulse */
    gpio_set_level(s_trig_gpio, 1);
    esp_rom_delay_us(10);
    gpio_set_level(s_trig_gpio, 0);

    /* Wait for echo to go high */
    int64_t t0 = esp_timer_get_time();
    while (gpio_get_level(s_echo_gpio) == 0) {
        if (esp_timer_get_time() - t0 > ECHO_TIMEOUT_US) {
            return -1.0f;
        }
    }

    /* Measure how long echo stays high */
    int64_t echo_start = esp_timer_get_time();
    while (gpio_get_level(s_echo_gpio) == 1) {
        if (esp_timer_get_time() - echo_start > ECHO_TIMEOUT_US) {
            return -1.0f;
        }
    }
    int64_t echo_end = esp_timer_get_time();

    float duration_us = (float)(echo_end - echo_start);
    /* distance = time * speed_of_sound / 2 (there and back) */
    return (duration_us * 0.0343f) / 2.0f;
}
