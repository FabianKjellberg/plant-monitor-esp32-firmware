#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "i2c.h"

#define BH1750_POWER_ON 0x01
#define BH1750_SINGLE_HI_RES 0x20

static i2c_master_dev_handle_t dev_light_handle = NULL;

esp_err_t bh1750_init(void) {
  i2c_master_bus_handle_t bus = i2c_get_bus_handle();
  
  i2c_device_config_t dev_cfg = {
    .device_address = 0x23,
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .scl_speed_hz = 100000,
  }; 

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev_light_handle));

  return ESP_OK;
}

esp_err_t bh1750_read_lux(float *lux) {
  uint8_t cmd = BH1750_POWER_ON;
  ESP_ERROR_CHECK(i2c_master_transmit(dev_light_handle, &cmd, 1, -1));

  cmd = BH1750_SINGLE_HI_RES;
  ESP_ERROR_CHECK(i2c_master_transmit(dev_light_handle, &cmd, 1, -1));

  vTaskDelay(pdMS_TO_TICKS(120));

  uint8_t res[2];
  ESP_ERROR_CHECK(i2c_master_receive(dev_light_handle, res, 2, -1));

  uint16_t raw = (res[0] << 8) | res[1];
  *lux = raw / 1.2f;

  return ESP_OK;
}

