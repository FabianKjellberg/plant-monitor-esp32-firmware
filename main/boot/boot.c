#include "boot.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#define uS_TO_S_FACTOR 1000000ULL
#define BOOT_GPIO_PIN GPIO_NUM_1


esp_err_t get_should_pair(bool *pair) {
    if (pair == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *pair = true;

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BOOT_GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE, 
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    int level;

    for (size_t i = 0; i < 50; i++)
    {
        level = gpio_get_level(BOOT_GPIO_PIN);
        if(level != 0) {
            *pair = false;
            return ESP_OK;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    printf("button held 5 sec, entering bluetooth pairing");
    return ESP_OK;
}

esp_err_t prepare_for_sleep(){
    uint64_t mask = (1ULL << BOOT_GPIO_PIN);

    esp_sleep_enable_ext1_wakeup_io(mask, ESP_EXT1_WAKEUP_ANY_LOW);
    esp_sleep_enable_timer_wakeup(5 * 60 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}