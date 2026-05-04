#include "boot.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"

#define BOOT_GPIO_PIN GPIO_NUM_9
#define HOLD_TIME 5000


esp_err_t get_should_pair(bool *pair) {
    if (pair == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    *pair = true;

    gpio_set_direction(BOOT_GPIO_PIN, GPIO_MODE_INPUT);

    int level;

    for (size_t i = 0; i < 5; i++)
    {
        level = gpio_get_level(BOOT_GPIO_PIN);
        if(level != 0) {
            *pair = false;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    return ESP_OK;
}