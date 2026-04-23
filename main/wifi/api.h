#ifndef API_H
#define API_H

#define MAC_ADDR_STR_LEN 18
#define TIMESTAMP_STR_LEN 32

typedef struct {
  char mac_addr[MAC_ADDR_STR_LEN];
  float lux;
  float temperature;
  float pressure;
  float humidity;
  int battery_mv;
  char read_at[TIMESTAMP_STR_LEN];
} sensor_reading_body;

void get_mac_addr(char *buffer, size_t size);

void post_sensor_reading(const sensor_reading_body *body);

#endif