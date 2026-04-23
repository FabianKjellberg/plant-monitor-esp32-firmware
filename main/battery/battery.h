#ifndef BATTERY_H
#define BATTERY_H

#include "esp_err.h"

esp_err_t battery_init(void);
esp_err_t battery_read(int *mv);

#endif