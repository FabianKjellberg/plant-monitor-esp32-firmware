#ifndef I2C_H
#define I2C_H

#include "esp_err.h"
#include "driver/i2c_master.h"

esp_err_t i2c_init(void);
esp_err_t i2c_print_devices(void);
i2c_master_bus_handle_t i2c_get_bus_handle(void);

#endif