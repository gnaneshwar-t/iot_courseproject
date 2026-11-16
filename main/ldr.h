#ifndef LDR_H
#define LDR_H

/**
 * @brief Configure ADC1 for the LDR voltage-divider input (GPIO34 / ADC1_CH6).
 */
void ldr_init(void);

/**
 * @brief Return the raw 12-bit ADC reading (0-4095).
 *        Whether a HIGHER reading means brighter or darker depends on
 *        how the LDR/resistor divider is wired - adjust automation.c
 *        thresholds to match your hardware.
 */
int ldr_read(void);

#endif // LDR_H
