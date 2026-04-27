#include <stdio.h>
#include <stdio.h>
#include "bh1750.h"
#include "bme280.h"
#include "wifi.h"
#include "battery.h"
#include "api.h"
#include "i2c.h"
#include "time_helper.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

#define uS_TO_S_FACTOR 1000000ULL
#define MAX_WIFI_CONNECTION_RETRY 20
#define DEBUG_MODE false

void app_main(void)
{
    printf("starting program\n");

    //turn on light (DEBUG)
    if(DEBUG_MODE) {
        xTaskCreate(led_task, "led_task", 4096, NULL, 5, NULL);
        led_set_state(LED_STATE_BLINKING_GREEN);
    }


    //init GPIO handles
    i2c_init();

    //create body to send
    sensor_reading_body api_body;

    //set mac adress
    get_mac_addr(api_body.mac_addr, MAC_ADDR_STR_LEN);

    //read battery
    battery_init();
    vTaskDelay(pdMS_TO_TICKS(50)); //give time for connector to settle. 
    battery_read(&api_body.battery_mv);

    //read BME280 sensors (temp, humid, press)
    bme280_init();
    bme280_data_t bme280_data;
    bme280_read_values(&bme280_data);
    api_body.humidity = bme280_data.humidity;
    api_body.pressure = bme280_data.pressure;
    api_body.temperature = bme280_data.temperature;

    //read bh1750 sensor (lux)
    bh1750_init();
    bh1750_read_lux(&api_body.lux);

    //start wifi chip, connect
    wifi_init();
    wifi_connect();
    
    int retries = 0; 
    while(!is_connected() && retries < MAX_WIFI_CONNECTION_RETRY) {
        printf("waiting for IP... (%d/%d)\n", retries, MAX_WIFI_CONNECTION_RETRY);
        vTaskDelay(pdMS_TO_TICKS(1000));
        retries++;
    }
    
    if(is_connected()) {
        printf("sending sensor readings to database\n");
        //read time
        time_get_iso8601(api_body.read_at, TIMESTAMP_STR_LEN);

        //send data to database
        post_sensor_reading(&api_body);
    }
    else {
        //handle in future? connect bluetooth? save to local storage?
    }

    if (DEBUG_MODE) {
        led_set_state(LED_STATE_OFF);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    //sleep
    printf("going to sleep\n");
    esp_sleep_enable_timer_wakeup(5 * 60 * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}
