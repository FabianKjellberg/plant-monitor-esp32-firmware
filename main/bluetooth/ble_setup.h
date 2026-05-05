#ifndef BLE_SETUP_H
#define BLE_SETUP_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BLE_STATUS_WAITING = 0,
    BLE_STATUS_SUCCESS = 1,
    BLE_STATUS_CONNECTING = 2,
    BLE_STATUS_FAILED = 3,
} ble_pairing_status_t;

esp_err_t ble_init(const char* mac_addr);

esp_err_t ble_wait_for_wifi_cred(void);

esp_err_t ble_set_pairing_status(ble_pairing_status_t status);

esp_err_t ble_stop();

#ifdef __cplusplus
}
#endif

#endif