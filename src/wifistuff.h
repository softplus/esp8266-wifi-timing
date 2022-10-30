/*
  Copyright (c) 2022-2022 John Mueller
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

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

