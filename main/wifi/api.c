#include <stdio.h>
#include <string.h>

#include "api.h"
#include "esp_err.h"
#include "esp_mac.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"

static const char *TAG = "API";
extern const char worker_ca_pem_start[] asm("_binary_worker_ca_pem_start");
extern const char worker_ca_pem_end[] asm("_binary_worker_ca_pem_end");

void get_mac_addr(char *buffer, size_t size) {
    uint8_t mac[6];

    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    snprintf(buffer, size,
             "%02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
}

void post_sensor_reading(const sensor_reading_body *body) {
    char json_body[256];

    int written = snprintf(
        json_body,
        sizeof(json_body),
        "{"
        "\"macAdress\":\"%s\","
        "\"humidity\":%.2f,"
        "\"lux\":%.2f,"
        "\"pressure\":%.2f,"
        "\"temperature\":%.2f,"
        "\"batteryMv\":%d,"
        "\"readTime\":\"%s\""
        "}",
        body->mac_addr,
        body->humidity,
        body->lux,
        body->pressure,
        body->temperature,
        body->battery_mv,
        body->read_at
    );

    if (written < 0 || written >= sizeof(json_body)) {
        ESP_LOGE(TAG, "JSON buffer too small or snprintf failed");
        return;
    }

    ESP_LOGI(TAG, "POST body: %s", json_body);

    esp_http_client_config_t config = {
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .url = "https://plant-monitor-worker-cf.tekital1000.workers.dev/upload",
        .cert_pem = worker_ca_pem_start,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return;
    }

    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, json_body, strlen(json_body));

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS POST status = %d",
                 esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTPS POST failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}