#include "time_helper.h"

#include <stdio.h>
#include <time.h>
#include "esp_sntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void time_sync_init(void) {
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

void time_wait_for_sync(void) {
  time_t now = 0;
  struct tm timeinfo = {0};

  int retry = 0;
  const int retry_count = 10;

  while (timeinfo.tm_year < (2020 - 1900) && retry < retry_count) {
    printf("Waiting for time sync...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    time(&now);
    gmtime_r(&now, &timeinfo);
    retry++;
  }
}

void time_get_iso8601(char *buffer, size_t size) {
  time_t now;
  struct tm timeinfo;

  time(&now);
  gmtime_r(&now, &timeinfo);
  strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
}