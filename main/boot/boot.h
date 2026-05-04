#ifndef BOOT_H
#define BOOT_H

#include "esp_err.h"
#include <stdbool.h>

esp_err_t get_should_pair(bool *pair);

#endif