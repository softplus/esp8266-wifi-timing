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

/* Handle wifi connection build-up in various options
 * Used for testing variations.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "main.h"
#include "secrets.h"
#include "settings.h"
#include "wifistuff.h"

/* Show the current connection information on Serial
 */
void show_connection(ESP8266WiFiClass *w) {
    #ifdef DEBUG_MODE
    Serial.print("WiFi Status - State:  "); Serial.println(w->status()); 
    Serial.print("  IP address:         "); Serial.print(w->localIP());
    Serial.print("  Gateway IP address: "); Serial.println(w->gatewayIP());
    Serial.print("  Subnet mask:        "); Serial.println(w->subnetMask());
    Serial.print("  DNS 0 IP address:   "); Serial.print(w->dnsIP(0));
    Serial.print("  DNS 1 IP address:   "); Serial.print(w->dnsIP(1));
    Serial.print("  DNS 2 IP address:   "); Serial.println(w->dnsIP(2));
    Serial.print("  BSSID:              "); Serial.print(w->BSSIDstr().c_str());
    Serial.print("  Channel: "); Serial.println(w->channel());
    #endif
}

/* Do a slow / connection 
 */
int wifi_slow_connect(ESP8266WiFiClass *w) {
	#define SLOW_TIMEOUT 10000 // ms
	w->mode(WIFI_STA);
	w->begin(WIFI_SSID, WIFI_AUTH);
	uint32_t timeout = millis() + SLOW_TIMEOUT;
	while ((w->status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
	return (w->status() == WL_CONNECTED);
}

/* To test just reconnecting without building a connection
 */
int wifi_just_reconnect(ESP8266WiFiClass *w) {
	uint32_t timeout = millis() + 5000; // max 5s
	w->mode(WIFI_STA);
	w->reconnect();
	while ((w->status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
	return (w->status() == WL_CONNECTED);
}

/* Try doing a fast connection, with cached BSSID, channel & persist
 */
int wifi_fast_connect(WIFI_SETTINGS_T *data, ESP8266WiFiClass *w) {
	#define FAST_TIMEOUT 5000 // ms
	// try fast connect
	w->persistent(true);
	w->mode(WIFI_STA);
	w->config(IPAddress(data->ip_address),
		IPAddress(data->ip_gateway), IPAddress(data->ip_mask), 
		IPAddress(data->ip_dns1), IPAddress(data->ip_dns2));
	w->begin(data->wifi_ssid, data->wifi_auth, data->wifi_channel, data->wifi_bssid, true);
	// wait for connection
	uint32_t timeout = millis() + FAST_TIMEOUT;
	//wifi_set_channel(ch);
	//wifi_station_connect();
	//w->reconnect();
	while ((w->status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
	if ((w->status() == WL_CONNECTED) && (w->channel() != data->wifi_channel)) {
		DEBUG_OUT("*** CHANNEL CHANGED *** **************************************");
		DEBUG_OUTS("Specified: "); DEBUG_OUTS(data->wifi_channel);
		DEBUG_OUTS(" - received: "); DEBUG_OUT(w->channel());
		w->printDiag(Serial);
	}
	return (w->status() == WL_CONNECTED);
}

/* Connect to this IP address & port; saves MQTT time
 */
int preconnect_ip(WiFiClient *wclient, IPAddress ip, int port) {
	#define PRECONNECT_TIMEOUT 5000
	uint32_t timeout = millis() + PRECONNECT_TIMEOUT;
	while ((!wclient->connect(ip, port)) && (millis()<timeout)) { delay(50); }
	return (wclient->connected());
}

/* Publish something to our MQTT server
 */
int publish_mqtt(WiFiClient *wclient, WIFI_SETTINGS_T *data, 
		const char *topic, const char *value) {
	// no timeouts no ragrets
	PubSubClient mqtt_client(*wclient);
	mqtt_client.setServer(data->mqtt_host_ip, data->mqtt_host_port);
	//mqtt_client.setServer(data->mqtt_host_str, data->mqtt_host_port);
	int status = false;
	if (mqtt_client.connect(MQTT_CLIENT_ID, data->mqtt_user, data->mqtt_auth)) {
		DEBUG_OUTS(topic); DEBUG_OUTS("=");DEBUG_OUT(value);
		if (strlen(topic)>1) {
			mqtt_client.publish(topic, value);
			mqtt_client.publish("wled/testing2", "VALUE2");
			mqtt_client.publish("wled/testing3", "VALUE3");
			mqtt_client.publish("wled/testing4", "VALUE4");
			mqtt_client.publish("wled/testing5", "VALUE5");
		}
		status = true;
	} else {
		status = false;
	}
	return status;
}
