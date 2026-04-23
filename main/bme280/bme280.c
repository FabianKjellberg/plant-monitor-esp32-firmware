#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "bme280.h"
#include "i2c.h"

typedef struct {
  uint16_t dig_T1;
  int16_t dig_T2;
  int16_t dig_T3;

  uint16_t dig_P1;
  int16_t dig_P2;
  int16_t dig_P3;
  int16_t dig_P4;
  int16_t dig_P5;
  int16_t dig_P6;
  int16_t dig_P7;
  int16_t dig_P8;
  int16_t dig_P9;

  uint8_t dig_H1;
  int16_t dig_H2;
  uint8_t dig_H3;
  int16_t dig_H4;
  int16_t dig_H5;
  int8_t dig_H6;

} bme280_calib_t;


static i2c_master_dev_handle_t dev_bme280_handle = NULL;
static bme280_calib_t bme280_calib;

esp_err_t bme280_write_reg(uint8_t reg, uint8_t value) {
  uint8_t data[] = {reg, value};

  ESP_ERROR_CHECK(i2c_master_transmit(dev_bme280_handle, data, 2, -1));

  return ESP_OK;
}

esp_err_t bme280_read_reg(uint8_t reg, uint8_t *value) {
  ESP_ERROR_CHECK(
    i2c_master_transmit_receive(dev_bme280_handle, &reg, 1, value, 1, -1)
  );

  return ESP_OK;
}

esp_err_t bme280_read_bytes(uint8_t reg, uint8_t *buf, size_t len) {
  ESP_ERROR_CHECK(
    i2c_master_transmit_receive(dev_bme280_handle, &reg, 1 , buf, len, -1)
  );

  return ESP_OK;
}

esp_err_t bme280_init(void) {
  i2c_master_bus_handle_t bus = i2c_get_bus_handle();

  i2c_device_config_t dev_cfg = {
    .device_address = 0x76,
    .dev_addr_length = I2C_ADDR_BIT_7,
    .scl_speed_hz = 100000,
  }; 

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev_bme280_handle));

  uint8_t reg_88 = 0x88;
  uint8_t buf_88_to_a1[25]; 
  bme280_read_bytes(reg_88, buf_88_to_a1, 25);

  uint8_t reg_e1 = 0xE1;
  uint8_t buf_e1_to_e7[7];
  bme280_read_bytes(reg_e1, buf_e1_to_e7, 7);

  bme280_calib.dig_T1 = ((uint16_t) buf_88_to_a1[1] << 8) | buf_88_to_a1[0];
  bme280_calib.dig_T2 = ((int16_t) buf_88_to_a1[3] << 8) | buf_88_to_a1[2];
  bme280_calib.dig_T3 = ((int16_t) buf_88_to_a1[5] << 8) | buf_88_to_a1[4];

  bme280_calib.dig_P1 = ((uint16_t) buf_88_to_a1[7] << 8) | buf_88_to_a1[6];
  bme280_calib.dig_P2 = ((int16_t) buf_88_to_a1[9] << 8) | buf_88_to_a1[8];
  bme280_calib.dig_P3 = ((int16_t) buf_88_to_a1[11] << 8) | buf_88_to_a1[10];
  bme280_calib.dig_P4 = ((int16_t) buf_88_to_a1[13] << 8) | buf_88_to_a1[12];
  bme280_calib.dig_P5 = ((int16_t) buf_88_to_a1[15] << 8) | buf_88_to_a1[14];
  bme280_calib.dig_P6 = ((int16_t) buf_88_to_a1[17] << 8) | buf_88_to_a1[16];
  bme280_calib.dig_P7 = ((int16_t) buf_88_to_a1[19] << 8) | buf_88_to_a1[18];
  bme280_calib.dig_P8 = ((int16_t) buf_88_to_a1[21] << 8) | buf_88_to_a1[20]; 
  bme280_calib.dig_P9 = ((int16_t) buf_88_to_a1[23] << 8) | buf_88_to_a1[22];

  bme280_calib.dig_H1 = buf_88_to_a1[24];
  bme280_calib.dig_H2 = ((int16_t)buf_e1_to_e7[1] << 8) | buf_e1_to_e7[0];
  bme280_calib.dig_H3 = buf_e1_to_e7[2];

  bme280_calib.dig_H4 =
    ((int16_t)buf_e1_to_e7[3] << 4) |
    (buf_e1_to_e7[4] & 0x0F);

  bme280_calib.dig_H5 =
    ((int16_t)buf_e1_to_e7[5] << 4) |
    ((buf_e1_to_e7[4] >> 4) & 0x0F);

  bme280_calib.dig_H6 = (int8_t)buf_e1_to_e7[6];

  return ESP_OK;
}

esp_err_t bme280_compensate_raw_values(bme280_data_t *data) {
  //temperature
  int32_t var1 = ((((int32_t)data->temperature_raw >> 3) - ((int32_t)bme280_calib.dig_T1 << 1)) * ((int32_t)bme280_calib.dig_T2)) >> 11;
  int32_t var2 = (((((int32_t)data->temperature_raw >> 4) - ((int32_t)bme280_calib.dig_T1)) * (((int32_t)data->temperature_raw >> 4) - ((int32_t)bme280_calib.dig_T1))) >> 12) * ((int32_t)bme280_calib.dig_T3) >> 14;
  int32_t t_fine = var1 + var2;

  data->temperature = ((t_fine * 5 + 128) >> 8) / 100.0f;
  //pressure
  int64_t var1_p, var2_p, p;
    var1_p = ((int64_t)t_fine) - 128000;
    var2_p = var1_p * var1_p * (int64_t)bme280_calib.dig_P6;
    var2_p = var2_p + ((var1_p * (int64_t)bme280_calib.dig_P5) << 17);
    var2_p = var2_p + (((int64_t)bme280_calib.dig_P4) << 35);
    var1_p = ((var1_p * var1_p * (int64_t)bme280_calib.dig_P3) >> 8) + ((var1_p * (int64_t)bme280_calib.dig_P2) << 12);
    var1_p = (((((int64_t)1) << 47) + var1_p) * (int64_t)bme280_calib.dig_P1) >> 33;

    if (var1_p == 0) {
        return ESP_FAIL;
    }

    p = 1048576 - data->pressure_raw;
    p = (((p << 31) - var2_p) * 3125) / var1_p;
    var1_p = (((int64_t)bme280_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2_p = (((int64_t)bme280_calib.dig_P8) * p) >> 19;
    p = ((p + var1_p + var2_p) >> 8) + (((int64_t)bme280_calib.dig_P7) << 4);

    data->pressure = (float)p / 256.0f;

    //humidity
    int32_t v_x1_u32r;
    v_x1_u32r = t_fine - ((int32_t)76800);
    v_x1_u32r = (((((data->humidity_raw << 14) - (((int32_t)bme280_calib.dig_H4) << 20) - (((int32_t)bme280_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme280_calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)bme280_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme280_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme280_calib.dig_H1)) >> 4);
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    data->humidity = (v_x1_u32r >> 12) / 1024.0f;

    return ESP_OK;
}

esp_err_t bme280_read_values(bme280_data_t *data) {
  uint8_t reg = 0xD0;
  uint8_t id = 0;

  bme280_read_reg(reg, &id);

  bme280_write_reg(0xE0, 0xB6);
  vTaskDelay(pdMS_TO_TICKS(10));

  bme280_write_reg(0xF2, 0x1);
  bme280_write_reg(0xF4, 0x25);

  vTaskDelay(pdMS_TO_TICKS(10));

  uint8_t buf[8];
  bme280_read_bytes(0xF7, buf, 8);

  data->pressure_raw = 
    ((uint32_t) buf[0] << 12) | 
    ((uint32_t) buf[1] << 4) | 
    ((uint32_t) buf[2] >> 4);

  data->temperature_raw = 
    ((uint32_t) buf[3] << 12) | 
    ((uint32_t) buf[4] << 4) | 
    ((uint32_t) buf[5] >> 4);

  data->humidity_raw = 
    ((uint16_t)buf[6] << 8) | 
    buf[7];

  bme280_compensate_raw_values(data);

  return ESP_OK;
}