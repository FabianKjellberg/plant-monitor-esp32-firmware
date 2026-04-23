#include "led_strip.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include <stdbool.h>

led_state_t led_state_current = LED_STATE_OFF;

led_strip_handle_t led_init(void) {
  led_strip_handle_t led_strip;
  
  led_strip_config_t led_config = {
    .strip_gpio_num = 8,
    .max_leds = 1,
    .led_model = LED_MODEL_WS2812,
    .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
  };

  led_strip_rmt_config_t rmt_config = {
    .resolution_hz = 10 * 1000 * 1000,
  };

  ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_config, &rmt_config, &led_strip));
  
  return led_strip;
}

void led_clear (led_strip_handle_t led_strip) {
  led_strip_set_pixel(led_strip, 0,0,0,0);
  led_strip_refresh(led_strip);
}

void led_set_state (led_state_t state) {
  led_state_current = state;
}

void led_task (void *pvParameters) {
  led_strip_handle_t led_strip = led_init();

  while (1) {
    switch (led_state_current)
    {
    case LED_STATE_BLINKING_BLUE:
      led_strip_set_pixel(led_strip, 0, 0, 0, 100);
      led_strip_refresh(led_strip);
      vTaskDelay(pdMS_TO_TICKS(50));
      led_clear(led_strip);
      vTaskDelay(pdMS_TO_TICKS(50));
      break;
    case LED_STATE_BLINKING_RED:
      led_strip_set_pixel(led_strip, 0, 100, 0, 0);
      led_strip_refresh(led_strip);
      vTaskDelay(pdMS_TO_TICKS(100));
      led_clear(led_strip);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
    case LED_STATE_BLINKING_GREEN:
      led_strip_set_pixel(led_strip, 0, 0, 100, 0);
      led_strip_refresh(led_strip);
      vTaskDelay(pdMS_TO_TICKS(100));
      led_clear(led_strip);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;

    case LED_STATE_BLINKING_PINK:
      led_strip_set_pixel(led_strip, 0, 100, 0, 50);
      led_strip_refresh(led_strip);
      vTaskDelay(pdMS_TO_TICKS(200));
      led_clear(led_strip);
      vTaskDelay(pdMS_TO_TICKS(800));
      break;
    
    case LED_STATE_OFF: 
    default:
      led_clear(led_strip);
      vTaskDelay(pdMS_TO_TICKS(100));
      break;
    }
    
  }
}