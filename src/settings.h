
#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

struct WIFI_SETTINGS_T {
	uint16_t magic;
	uint32_t ip_address;
	uint32_t ip_gateway;
	uint32_t ip_mask;
	uint32_t ip_dns1;
	uint32_t ip_dns2;
	char wifi_ssid[50];
	char wifi_auth[50];
	uint8_t wifi_bssid[6];
	uint8_t wifi_channel;
	char mqtt_host_str[50];
	uint32_t mqtt_host_ip;
	uint16_t mqtt_host_port;
	char mqtt_user[50];
	char mqtt_auth[50];
	uint8_t force_slow;
};

const uint16_t MAGIC_NUM = 0x1AC3;

void build_settings_from_wifi(WIFI_SETTINGS_T *data, ESP8266WiFiClass *w);
void save_settings_to_flash(WIFI_SETTINGS_T *data);
int get_settings_from_flash(WIFI_SETTINGS_T *data);
void display_settings(WIFI_SETTINGS_T *data);

#endif
