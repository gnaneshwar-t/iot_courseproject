#ifndef HCSR04_H
#define HCSR04_H

/**
 * @brief Configure the trigger (output) and echo (input) GPIOs.
 *        NOTE: the HC-SR04 ECHO pin outputs ~5V. ESP32 GPIOs are 3.3V
 *        logic - use a resistor voltage divider (or level shifter) on
 *        ECHO before connecting it to the ESP32.
 */
void hcsr04_init(int trig_gpio, int echo_gpio);

/**
 * @brief Trigger a measurement and return distance in centimeters.
 *        Returns -1.0f on timeout / no echo detected.
 */
float hcsr04_get_distance_cm(void);

#endif // HCSR04_H
