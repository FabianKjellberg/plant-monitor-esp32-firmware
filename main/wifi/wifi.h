#ifndef WIFI_H
#define WIFI_H

#include "esp_err.h"

esp_err_t wifi_init(void); 
esp_err_t wifi_connect(void);
bool is_connected(void);
bool is_provisioned(void);

#endif