#ifndef BME280_H
#define BME280_H

#include "esp_err.h"

typedef struct {
    uint32_t pressure_raw;
    uint32_t temperature_raw;
    uint16_t humidity_raw;

    float temperature;
    float pressure;
    float humidity;
} bme280_data_t;

esp_err_t bme280_init(void);
esp_err_t bme280_read_values(bme280_data_t *data);

#endif