
#ifndef WIFISTUFF_H
#define WIFISTUFF_H

#include <Arduino.h>
#include <ESP8266WiFi.h>

void show_connection(ESP8266WiFiClass *w);
int wifi_just_reconnect(ESP8266WiFiClass *w);
int wifi_slow_connect(ESP8266WiFiClass *w);
int wifi_fast_connect(WIFI_SETTINGS_T *data, ESP8266WiFiClass *w);
int preconnect_ip(WiFiClient *wclient, IPAddress ip, int port);
int publish_mqtt(WiFiClient *wclient, WIFI_SETTINGS_T *data, 
    const char *topic, const char *value);

#endif

