#ifndef DHT11_H
#define DHT11_H

#include <stdbool.h>

typedef struct {
    float temperature;   // degrees C
    float humidity;      // % RH
    bool  valid;          // true if last read passed checksum
} dht11_data_t;

/**
 * @brief Configure the GPIO used for the DHT11 single-wire bus.
 */
void dht11_init(int gpio_num);

/**
 * @brief Perform a blocking read of the DHT11 sensor.
 *        Do not call this more often than once every ~2 seconds
 *        (DHT11 hardware limitation).
 */
dht11_data_t dht11_read(void);

#endif // DHT11_H
