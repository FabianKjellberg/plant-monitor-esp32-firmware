file(GLOB_RECURSE SOURCES "*.c" "*.cpp")

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "." "battery" "bh1750" "bme280" "helpers" "i2c" "wifi" "led" "boot" "bluetooth"
    REQUIRES 
        driver
        esp_wifi
        esp_driver_i2c
        esp_driver_gpio
        esp_adc
        nvs_flash
        esp_http_client
        mbedtls
        led_strip    
        bt
        esp-nimble-cpp
        nvs_flash
    EMBED_TXTFILES "certs/worker_ca.pem"
)
