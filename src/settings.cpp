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

/* Handle saving & restoring settings from Flash
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include "main.h"
#include "settings.h"
#include "secrets.h"

void build_settings_from_wifi(WIFI_SETTINGS_T *data, ESP8266WiFiClass *w) {
    // main settings
    data->magic = MAGIC_NUM;
    data->ip_address = w->localIP();
    data->ip_gateway = w->gatewayIP();
    data->ip_mask = w->subnetMask();
    data->ip_dns1 = w->dnsIP(0);
    data->ip_dns2 = w->dnsIP(1);
    strncpy(data->wifi_ssid, WIFI_SSID, 50);
    strncpy(data->wifi_auth, WIFI_AUTH, 50);
    memcpy(data->wifi_bssid, w->BSSID(), 6);
    data->wifi_channel = w->channel();
    // lookup IP for mqtt server
    strncpy(data->mqtt_host_str, MQTT_SERVER, 50);
    IPAddress mqtt_ip;
    int err = w->hostByName(MQTT_SERVER, mqtt_ip);
    if (err==0) { 
    #ifdef DEBUG_MODE
    Serial.print(" ** Can't resolve host "); Serial.println(MQTT_SERVER); 
    #endif
    }
    memcpy(&data->mqtt_host_ip, mqtt_ip, 4);
    strncpy(data->mqtt_auth, MQTT_AUTH, 50);
    strncpy(data->mqtt_user, MQTT_USER, 50);
    data->mqtt_host_port = MQTT_SERVER_PORT;
}


void save_settings_to_flash(WIFI_SETTINGS_T *data) {
	char buf[sizeof(WIFI_SETTINGS_T)];
	memcpy(&buf, data, sizeof(WIFI_SETTINGS_T));
	EEPROM.begin(sizeof(WIFI_SETTINGS_T));
	EEPROM.put(0, buf);
	EEPROM.end();
}

int get_settings_from_flash(WIFI_SETTINGS_T *data) { // dunno why not parameters
	char buf[sizeof(WIFI_SETTINGS_T)];
	EEPROM.begin(sizeof(WIFI_SETTINGS_T));
	EEPROM.get(0, buf);
	EEPROM.end();
	memcpy((char *)data, buf, sizeof(WIFI_SETTINGS_T));
	return (data->magic == MAGIC_NUM);
}

void display_settings(WIFI_SETTINGS_T *data) {
	#ifdef DEBUG_MODE
	char buf[100];
	Serial.println("Settings:");
	Serial.print("Magic:       "); 
	sprintf(buf, "%04X", data->magic); Serial.println(buf);
	Serial.print("Local IP:    "); 
	sprintf(buf, "%08X", data->ip_address); Serial.println(buf);
	Serial.print("Gateway IP:  "); 
	sprintf(buf, "%08X", data->ip_gateway); Serial.println(buf);
	Serial.print("Mask:        "); 
	sprintf(buf, "%08X", data->ip_mask); Serial.println(buf);
	Serial.print("DNS 1 IP:    "); 
	sprintf(buf, "%08X", data->ip_dns1); Serial.println(buf);
	Serial.print("DNS 2 IP:    "); 
	sprintf(buf, "%08X", data->ip_dns2); Serial.println(buf);
	Serial.print("Wifi SSID:   "); Serial.println(data->wifi_ssid);
	Serial.print("Wifi Auth:   "); Serial.println(data->wifi_auth);
	Serial.print("Wifi BSSID:  "); 
	sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", 
		data->wifi_bssid[0], data->wifi_bssid[1], data->wifi_bssid[2], 
		data->wifi_bssid[3], data->wifi_bssid[4], data->wifi_bssid[5]); Serial.println(buf);
	Serial.print("Wifi Channel:"); sprintf(buf, "%d", data->wifi_channel); Serial.println(buf);
	Serial.print("MQTT Host:   "); Serial.println(data->mqtt_host_str);
	Serial.print("MQTT IP:     "); sprintf(buf, "%08X", data->mqtt_host_ip); Serial.println(buf);
	Serial.print("MQTT Port:   "); sprintf(buf, "%d", data->mqtt_host_port); Serial.println(buf);
	Serial.print("MQTT User:   "); Serial.println(data->mqtt_user);
	Serial.print("MQTT Pass:   "); Serial.println(data->mqtt_auth);
	#endif
}