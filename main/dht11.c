#include "dht11.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "esp_log.h"

static const char *TAG = "DHT11";
static int s_gpio;

void dht11_init(int gpio_num)
{
    s_gpio = gpio_num;
    gpio_reset_pin(s_gpio);
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(s_gpio, 1);
}

/* Busy-wait until the pin reaches `level`. Returns elapsed microseconds,
 * or -1 if `timeout_us` is exceeded (used for bit-timing decode). */
static int wait_for_level(int level, int timeout_us)
{
    int elapsed = 0;
    while (gpio_get_level(s_gpio) != level) {
        esp_rom_delay_us(1);
        if (++elapsed > timeout_us) {
            return -1;
        }
    }
    return elapsed;
}

dht11_data_t dht11_read(void)
{
    dht11_data_t result = { .temperature = 0, .humidity = 0, .valid = false };
    uint8_t data[5] = { 0 };

    /* --- Host start signal --- */
    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(s_gpio, 0);
    esp_rom_delay_us(18000);   // >= 18ms low
    gpio_set_level(s_gpio, 1);
    esp_rom_delay_us(30);
    gpio_set_direction(s_gpio, GPIO_MODE_INPUT);

    /* --- Sensor response: 80us low, 80us high --- */
    if (wait_for_level(0, 100) < 0) { ESP_LOGW(TAG, "No response (low)"); return result; }
    if (wait_for_level(1, 100) < 0) { ESP_LOGW(TAG, "No response (high)"); return result; }
    if (wait_for_level(0, 100) < 0) { ESP_LOGW(TAG, "No response (start bit)"); return result; }

    /* --- 40 data bits: each bit = 50us low + (26-28us=0 / 70us=1) high --- */
    for (int i = 0; i < 40; i++) {
        if (wait_for_level(1, 100) < 0) { ESP_LOGW(TAG, "Timeout: bit start"); return result; }
        int high_us = wait_for_level(0, 100);
        if (high_us < 0) { ESP_LOGW(TAG, "Timeout: bit end"); return result; }

        data[i / 8] <<= 1;
        if (high_us > 40) {
            data[i / 8] |= 1;
        }
    }

    uint8_t checksum = (uint8_t)(data[0] + data[1] + data[2] + data[3]);
    if (checksum != data[4]) {
        ESP_LOGW(TAG, "Checksum mismatch (got 0x%02X, expected 0x%02X)", data[4], checksum);
        return result;
    }

    result.humidity    = (float)data[0];
    result.temperature = (float)data[2];
    result.valid = true;

    gpio_set_direction(s_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(s_gpio, 1);

    return result;
}
