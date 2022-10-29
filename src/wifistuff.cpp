
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "main.h"
#include "secrets.h"
#include "settings.h"
#include "wifistuff.h"

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

int wifi_slow_connect(ESP8266WiFiClass *w) {
	#define SLOW_TIMEOUT 10000 // ms
	w->mode(WIFI_STA);
	w->begin(WIFI_SSID, WIFI_AUTH);
	uint32_t timeout = millis() + SLOW_TIMEOUT;
	while ((w->status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
	return (w->status() == WL_CONNECTED);
}

int wifi_just_reconnect(ESP8266WiFiClass *w) {
	uint32_t timeout = millis() + 5000; // max 5s
	w->mode(WIFI_STA);
	w->reconnect();
	while ((w->status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
	return (w->status() == WL_CONNECTED);
}

int wifi_fast_connect(WIFI_SETTINGS_T *data, ESP8266WiFiClass *w) {
	#define FAST_TIMEOUT 5000 // ms
	// try fast connect
	w->persistent(true); // from "g"
	w->mode(WIFI_STA);
	w->config(IPAddress(data->ip_address),
		IPAddress(data->ip_gateway), IPAddress(data->ip_mask), 
		IPAddress(data->ip_dns1), IPAddress(data->ip_dns2));
	//w->begin(data->wifi_ssid, data->wifi_auth, 
	//	data->wifi_channel, data->wifi_bssid, false);
	int ch = data->wifi_channel;
	//ch=6;
	//w->begin(data->wifi_ssid, data->wifi_auth, ch, data->wifi_bssid, false);
	w->begin(data->wifi_ssid, data->wifi_auth, ch, data->wifi_bssid, true);
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

int preconnect_ip(WiFiClient *wclient, IPAddress ip, int port) {
	#define PRECONNECT_TIMEOUT 5000
	uint32_t timeout = millis() + PRECONNECT_TIMEOUT;
	while ((!wclient->connect(ip, port)) && (millis()<timeout)) { delay(50); }
	return (wclient->connected());
}


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
