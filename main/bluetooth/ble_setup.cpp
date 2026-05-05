#include "esp_err.h"
#include "ble_setup.h"
#include <NimBLEDevice.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_WIFI "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_MAC "d2ed6f67-a068-4501-8b06-44e21ed90382"
#define CHARACTERISTIC_UUID_STATUS "e3237190-2c7b-449e-862d-6060c410313a"

SemaphoreHandle_t ble_wifi_cred_semaphore = NULL;
NimBLECharacteristic* pCharStatus;
ble_pairing_status_t current_ble_status;

class BleEvents : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharactaristics, NimBLEConnInfo& connInfo) override {
        if(current_ble_status == BLE_STATUS_CONNECTING || current_ble_status == BLE_STATUS_SUCCESS) {
            printf("device busy or succeeded");
            return;
        }
        
        std::string rxValue = pCharactaristics->getValue();

        printf("RX length: %u\n", (unsigned int)rxValue.length());

        for (size_t i = 0; i < rxValue.length(); i++) {
            printf("%02X ", (unsigned char)rxValue[i]);
        }
        printf("\n");

        if(rxValue.length() > 0) {
            size_t del_pos = rxValue.find('|');
            if (del_pos != std::string::npos) {
                std::string ssid = rxValue.substr(0, del_pos);
                std::string pass = rxValue.substr(del_pos + 1);

                nvs_handle_t nvs_handle;
                esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);

                if(err == ESP_OK){
                    nvs_set_str(nvs_handle, "wifi_ssid", ssid.c_str());
                    nvs_set_str(nvs_handle, "wifi_pass", pass.c_str());

                    nvs_commit(nvs_handle);
                    nvs_close(nvs_handle);

                    xSemaphoreGive(ble_wifi_cred_semaphore);
                }
                else {
                    printf("Error opening namespace for flash");
                }
            }
        }
    }
};

static BleEvents bleCallbacks;

esp_err_t ble_init(const char* mac_addr) {

    ble_wifi_cred_semaphore = xSemaphoreCreateBinary();

    NimBLEDevice::init("plant-monitor-pot");

    NimBLEServer *pServer = NimBLEDevice::createServer();
    NimBLEService *pService = pServer->createService(SERVICE_UUID);
    NimBLECharacteristic *pCharMac = pService->createCharacteristic(CHARACTERISTIC_UUID_MAC, NIMBLE_PROPERTY::READ);

    pCharMac->setValue(mac_addr);

    pCharStatus = pService->createCharacteristic(CHARACTERISTIC_UUID_STATUS, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::READ);
    ble_set_pairing_status(BLE_STATUS_WAITING);

    NimBLECharacteristic *pCharWifi = pService->createCharacteristic(CHARACTERISTIC_UUID_WIFI, NIMBLE_PROPERTY::WRITE);

    pCharWifi->setCallbacks(&bleCallbacks);

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    printf("bluetooth waiting for connecting devices...");

    return ESP_OK;
}

esp_err_t ble_wait_for_wifi_cred(void) {
    if(ble_wifi_cred_semaphore == NULL) {
        printf("Error: Bluetooth is not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(ble_wifi_cred_semaphore, portMAX_DELAY) == pdTRUE) {
        return ESP_OK;
    }

    return ESP_FAIL;
}


esp_err_t ble_set_pairing_status(ble_pairing_status_t status) {
    if(pCharStatus == NULL) {
        printf("Error: Bluetooth is not initialised");
        return ESP_ERR_INVALID_STATE;
    }

    current_ble_status = status;

    char statusStr[2];
    snprintf(statusStr, sizeof(statusStr), "%d", status);

    pCharStatus->setValue(statusStr);
    pCharStatus->notify();

    return ESP_OK;
}

esp_err_t ble_stop(void) {
    NimBLEDevice::getAdvertising()->stop();

    return ESP_OK;
}