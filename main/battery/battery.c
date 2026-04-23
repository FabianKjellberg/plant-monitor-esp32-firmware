#include "esp_err.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t cali_handle = NULL;

esp_err_t battery_init(void) {
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  esp_err_t ret = adc_oneshot_new_unit(&init_config1, &adc_handle);
  if (ret != ESP_OK) return ret;

  adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BITWIDTH_DEFAULT,
    .atten = ADC_ATTEN_DB_12,
  };
  ret = adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &config);
  if (ret != ESP_OK) return ret;

  adc_cali_curve_fitting_config_t cali_config = {
    .unit_id = ADC_UNIT_1,
    .chan = ADC_CHANNEL_0,      
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_DEFAULT,
  };
  
  return adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle);
}

esp_err_t battery_read(int *v) {
  int warm_up;
  int voltage_mv = 0;
  int accumulated_voltage_mv = 0;
  const int iterations = 16;

  //warmup
  adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &warm_up);
  vTaskDelay(pdMS_TO_TICKS(10));

  for (int i = 0; i < iterations; i++) {
    adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &voltage_mv);
    adc_cali_raw_to_voltage(cali_handle, voltage_mv, v);

    accumulated_voltage_mv += voltage_mv;
  }
  int avg_at_pin = accumulated_voltage_mv / 16;
  *v = (avg_at_pin * 122) / 22;

  return ESP_OK;
}
