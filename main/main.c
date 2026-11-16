#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "dht11.h"
#include "ldr.h"
#include "hcsr04.h"
#include "relay.h"
#include "wifi_manager.h"
#include "http_server.h"
#include "automation.h"

/* ---- Pin configuration (change to match your wiring) ---- */
#define DHT11_GPIO        4
#define HCSR04_TRIG_GPIO  5
#define HCSR04_ECHO_GPIO  18   // through a voltage divider, ECHO is 5V!
#define RELAY_LIGHT_GPIO  19
#define RELAY_FAN_GPIO    21

/* ---- Wi-Fi credentials ---- */
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

/* ---- Loop timing ---- */
#define LOOP_DELAY_MS   2000   // DHT11 needs >=1s between reads

void app_main(void)
{
    /* --- Peripheral init --- */
    dht11_init(DHT11_GPIO);
    ldr_init();
    hcsr04_init(HCSR04_TRIG_GPIO, HCSR04_ECHO_GPIO);
    relay_init(RELAY_LIGHT_GPIO, RELAY_FAN_GPIO);
    automation_init();

    /* --- Wi-Fi + web server --- */
    wifi_manager_init_sta(WIFI_SSID, WIFI_PASSWORD);
    if (wifi_manager_is_connected()) {
        http_server_start();
        printf("Web interface ready - open http://<device-ip>/ in a browser.\n");
    } else {
        printf("Wi-Fi not connected - continuing with sensors/relays only.\n");
    }

    /* --- Single main loop: read sensors, run automation, wait --- */
    while (1) {
        dht11_data_t dht      = dht11_read();
        int          light    = ldr_read();
        float        distance = hcsr04_get_distance_cm();

        if (dht.valid) {
            automation_run(dht.temperature, dht.humidity, light, distance);
        } else {
            printf("DHT11 read failed, keeping previous relay states.\n");
        }

        printf("Temp: %.1f C | Hum: %.1f %% | Light: %d | Dist: %.1f cm | "
               "Light relay: %s | Fan relay: %s | Mode: %s\n",
               dht.temperature, dht.humidity, light, distance,
               relay_get_light_state() ? "ON" : "OFF",
               relay_get_fan_state() ? "ON" : "OFF",
               automation_get_mode() ? "AUTO" : "MANUAL");

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY_MS));
    }
}
