#include "esp_err.h"
#include "driver/i2c_master.h"

static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t i2c_init(void) {
  i2c_master_bus_config_t i2c_config_t = {
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = -1,
    .scl_io_num = 22,
    .sda_io_num = 21,
    .glitch_ignore_cnt = 7,
    .flags.enable_internal_pullup = true,
  };

  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config_t, &bus_handle));

  return ESP_OK;
}

i2c_master_bus_handle_t i2c_get_bus_handle(void) {
  return bus_handle;
}

esp_err_t i2c_print_devices(void) {
  i2c_master_bus_handle_t bus = i2c_get_bus_handle();
  if (bus == NULL) {
    return ESP_ERR_INVALID_STATE;
  }

  for (uint8_t addr = 0x08; addr < 0x78; addr++) {
    esp_err_t err = i2c_master_probe(bus, addr, 100);
    if (err == ESP_OK) {
      printf("Found device at 0x%02X\n", addr);
    } else if (err == ESP_ERR_TIMEOUT) {
      printf("Bus timeout at 0x%02X\n", addr);
    }
  }

  return ESP_OK;
}
