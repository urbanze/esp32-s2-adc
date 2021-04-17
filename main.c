#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/dac.h"
#include <esp_adc_cal.h>
//#include "driver/esp32s2/include/driver/adc.h"

esp_adc_cal_characteristics_t adc_cal;

//Inicia o pino de ADC
void init_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_13);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

    esp_adc_cal_value_t adc_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_13, 1100, &adc_cal);

    if (adc_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
	{
		ESP_LOGI(__func__, "Vref eFuse encontrado: %umV", adc_cal.vref);
	}
	else if (adc_type == ESP_ADC_CAL_VAL_EFUSE_TP)
	{
		ESP_LOGI(__func__, "Two Point eFuse encontrado");
	}
	else
	{
		ESP_LOGW(__func__, "Nada encontrado, utilizando Vref padrao: %umV", adc_cal.vref);
	}
}

//Faz a media de leituras no pino e mostra na tela
int adc_read()
{
    uint32_t raw = 0;
    for (int i = 0; i < 100; i++)
    {
        raw += adc1_get_raw(ADC1_CHANNEL_0);
        ets_delay_us(30);
    }
    raw /= 100;
    
    int vraw = (int)(raw * (3.3/8192.0) * 1000.0);//Valor RAW
    int vcal = esp_adc_cal_raw_to_voltage(raw, &adc_cal);//Valor CALIBRADO pela IDF

    ESP_LOGI(__func__, "RAW: %d, CAL: %d", vraw, vcal);
    return vcal;
}


void app_main(void)
{
    dac_output_enable(DAC_CHANNEL_1);
    init_adc();

    for (int i = 0; i < 256; i++)
    {
        dac_output_voltage(DAC_CHANNEL_1, i);

        vTaskDelay(pdMS_TO_TICKS(10));

        adc_read();
    }

    dac_output_voltage(DAC_CHANNEL_1, 1);
}

