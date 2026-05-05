#include <stdbool.h>
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "time_helper.h"
#include "nvs.h"
#include "nvs_flash.h"

static bool has_ip = false;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  printf("event_base: %s, event_id: %ld\n", event_base, event_id);
  
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *) event_data;
    printf("Disconnected, reason=%d\n", event->reason);

    has_ip = false;
    esp_wifi_connect(); 
  }

  if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    time_sync_init();
    time_wait_for_sync();
    has_ip = true;
  }
}

esp_err_t wifi_init(void) {  
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  esp_netif_t *netif = esp_netif_create_default_wifi_sta();
  esp_netif_set_hostname(netif, "Plant Monitor");

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_err_t ret2 = esp_wifi_init(&cfg);
  if (ret2 != ESP_OK) {
    return ret2;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

  printf("Wi-Fi driver started successfully!\n");
  return ESP_OK;
}

esp_err_t wifi_connect() {
  
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);

  if(err != ESP_OK) {
    return err;
  }

  char ssid[33];
  char pass[65];
  size_t ssid_len = sizeof(ssid);
  size_t pass_len = sizeof(pass);

  nvs_get_str(nvs_handle, "wifi_ssid", ssid, &ssid_len);
  nvs_get_str(nvs_handle, "wifi_pass", pass, &pass_len);

  nvs_close(nvs_handle);
  
  wifi_config_t wifi_cfg = {0};
  
  strncpy((char*) wifi_cfg.sta.ssid, ssid, sizeof(wifi_cfg.sta.ssid));
  strncpy((char*) wifi_cfg.sta.password, pass, sizeof(wifi_cfg.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
  ESP_ERROR_CHECK(esp_wifi_connect());

  return ESP_OK;
}

bool is_connected() {
  return has_ip;
}

bool is_provisioned() {
  nvs_handle_t nvs_handle;

  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) return false;

  size_t required_size = 0;

  esp_err_t res = nvs_get_str(nvs_handle, "wifi_ssid", NULL, &required_size);
    
  nvs_close(nvs_handle);
  return (res == ESP_OK && required_size > 0);
}


