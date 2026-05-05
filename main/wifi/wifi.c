#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "time_helper.h"

#define MAX_WIFI_RETRIES 5

static bool has_ip = false;
static int wifi_retry_count = 0;
static bool time_synced = false;

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
) {
    printf("event_base: %s, event_id: %ld\n", event_base, event_id);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *event =
            (wifi_event_sta_disconnected_t *) event_data;

        printf("Disconnected, reason=%d\n", event->reason);

        has_ip = false;
        time_synced = false;

        if (wifi_retry_count < MAX_WIFI_RETRIES) {
            wifi_retry_count++;

            printf(
                "Retrying Wi-Fi... attempt %d/%d\n",
                wifi_retry_count,
                MAX_WIFI_RETRIES
            );

            esp_err_t err = esp_wifi_connect();

            if (err != ESP_OK) {
                printf("esp_wifi_connect failed: %s\n", esp_err_to_name(err));
            }
        } else {
            printf("Wi-Fi failed after %d retries\n", MAX_WIFI_RETRIES);
        }

        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;

        printf("Got IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));

        has_ip = true;
        wifi_retry_count = 0;

        if (!time_synced) {
            time_sync_init();
            time_wait_for_sync();
            time_synced = true;
        }

        return;
    }
}

esp_err_t wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());

    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        return err;
    }

    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    if (netif == NULL) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_netif_set_hostname(netif, "Plant-Monitor"));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        return err;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("Wi-Fi driver started successfully!\n");

    return ESP_OK;
}

esp_err_t wifi_connect(void) {
    nvs_handle_t nvs_handle;

    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        printf("Failed to open NVS: %s\n", esp_err_to_name(err));
        return err;
    }

    char ssid[33] = {0};
    char pass[65] = {0};

    size_t ssid_len = sizeof(ssid);
    size_t pass_len = sizeof(pass);

    err = nvs_get_str(nvs_handle, "wifi_ssid", ssid, &ssid_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        printf("Failed to read wifi_ssid: %s\n", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_str(nvs_handle, "wifi_pass", pass, &pass_len);
    if (err != ESP_OK) {
        nvs_close(nvs_handle);
        printf("Failed to read wifi_pass: %s\n", esp_err_to_name(err));
        return err;
    }

    nvs_close(nvs_handle);

    wifi_config_t wifi_cfg = {0};

    strncpy((char *) wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid) - 1);
    strncpy((char *) wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password) - 1);

    wifi_retry_count = 0;
    has_ip = false;
    time_synced = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        printf("Initial esp_wifi_connect failed: %s\n", esp_err_to_name(err));
        return err;
    }

    return ESP_OK;
}

bool is_connected(void) {
    return has_ip;
}

bool is_provisioned(void) {
    nvs_handle_t nvs_handle;

    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        return false;
    }

    size_t required_size = 0;

    err = nvs_get_str(nvs_handle, "wifi_ssid", NULL, &required_size);

    nvs_close(nvs_handle);

    return err == ESP_OK && required_size > 1;
}