#ifndef H_LED
#define H_LED

#include "led_strip.h"
#include "esp_err.h"

typedef enum {
  LED_STATE_OFF,
  LED_STATE_BLINKING_RED,
  LED_STATE_BLINKING_BLUE, 
  LED_STATE_BLINKING_GREEN,
  LED_STATE_BLINKING_PINK,
} led_state_t;

extern led_state_t led_state_current;

void led_set_state(led_state_t state);
void led_task(void *pvParameters);
void led_clear(led_strip_handle_t led_strip);

#endif