#include "ldr.h"
#include "driver/adc.h"

/* GPIO34 = ADC1_CHANNEL_6 on the ESP32 */
#define LDR_ADC_CHANNEL ADC1_CHANNEL_6

void ldr_init(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(LDR_ADC_CHANNEL, ADC_ATTEN_DB_11);
}

int ldr_read(void)
{
    return adc1_get_raw(LDR_ADC_CHANNEL);
}
