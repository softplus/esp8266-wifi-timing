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

/* Main functions for ESP8266 fast wifi connection testing
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include "main.h"
#include "times.h"
#include "secrets.h"
#include "settings.h"
#include "wifistuff.h"

#define MQTT_ACTION_TOPIC "wled/testing"
#define MQTT_ACTION_VALUE "T"

struct WIFI_SETTINGS_T wifi_settings;

#define TRY_FASTCONNECT
//#define TRY_AUTORECONNECT
//#define TRY_ENABLESTA
//#define TRY_USERECONNECT
#define TRY_STATICIP

void setup() {
	// requirements
	WiFi.setAutoConnect(false);

	Serial.begin(115200);
	delay(1500); // wait some secs

	// show settings
	DEBUG_OUT("<start>");
	DEBUG_OUTS("<strategy=");
	DEBUG_OUTS("ch:?t,esp01,nodebug,");
	DEBUG_OUTS("sdk-default,")
	#ifdef TRY_FASTCONNECT
	DEBUG_OUTS("fastconnect,");
	#else
	DEBUG_OUTS("no-settings,");
	#endif
	#ifdef TRY_AUTORECONNECT
	DEBUG_OUTS("autoreconnect,");
	#endif
	#ifdef TRY_ENABLESTA
	DEBUG_OUTS("enablesta,");
	#endif
	#ifdef TRY_USERECONNECT
	DEBUG_OUTS("usereconnect,");
	#endif
	#ifdef TRY_STATICIP
	DEBUG_OUTS("staticip,");
	#endif
	
	DEBUG_OUT(">");

	uint32_t start_time_all = millis();

	TIME_START(ts_setup_total);
	TIME_START(ts_setup_wifi);

	bool wifi_working = false;
	bool save_wifi_settings = false;

	DEBUG_OUT("get_settings_from_flash");

	TIME_START(ts_get_flash);
	bool data_ok = get_settings_from_flash(&wifi_settings);
	TIME_STOP(ts_get_flash, "get_flash");

	#ifdef TRY_FASTCONNECT
	if ((!data_ok) || (wifi_settings.force_slow!=0)) {
		DEBUG_OUTS("<slow_reason="); 
		DEBUG_OUTS(data_ok?"forced":"settings_bad");
		DEBUG_OUT(">");

		DEBUG_OUT("doing slow connect");

		TIME_START(ts_slow_1);
		bool slow_ok = wifi_slow_connect(&WiFi);
		TIME_STOP(ts_slow_1, "slow_connect_1");
		DEBUG_OUT("<wifi_conn=slow>")

		if (!slow_ok) {
			wifi_working = false;
			DEBUG_OUT(" Failed ");
		} else {
			// connected, cache settings
			wifi_settings.force_slow = 0;
			save_wifi_settings = true;
			wifi_working = true;
		}
	} else {
		// try fast-connect
		//display_settings(&wifi_settings);
		DEBUG_OUT("Try wifi_fast_connect");

		TIME_START(ts_wifi_fast);
		bool can_fast = wifi_fast_connect(&wifi_settings, &WiFi);
		TIME_STOP(ts_wifi_fast, "wifi_fast_connect");

		if (!can_fast) { 
			// nope, revert to slow
			DEBUG_OUT("<wifi_conn=fallback_slow>")
			DEBUG_OUT("Try fallback wifi_slow_connect...");
			TIME_START(ts_slow_2);
			bool try_slow = wifi_slow_connect(&WiFi);
			TIME_STOP(ts_slow_2, "fallback_slow_connect");

			if (!try_slow) {
				wifi_working = false; // we failed. sad
				DEBUG_OUT("Slow connect fallback failed");
			} else {
				wifi_working = true;
				save_wifi_settings = true;
			}
		} else {
			DEBUG_OUT("<wifi_conn=fast>")
			wifi_working = true;
		}
	}
	#else
		DEBUG_OUT("Try connection...");
		#ifdef TRY_ENABLESTA
		WiFi.enableSTA(true);
		#endif
		#ifdef TRY_AUTORECONNECT
	    WiFi.setAutoReconnect(true);
		//WiFi.setAutoConnect(true);
		WiFi.persistent(true);
		#endif

		#ifdef TRY_STATICIP
		WiFi.config(IPAddress(wifi_settings.ip_address),
			IPAddress(wifi_settings.ip_gateway), 
			IPAddress(wifi_settings.ip_mask), 
			IPAddress(wifi_settings.ip_dns1), 
			IPAddress(wifi_settings.ip_dns2));
		#endif

		bool recon_ok = false;
		#ifdef TRY_USERECONNECT
		if (!wifi_settings.force_slow) {
			TIME_START(ts_recon);
			recon_ok = wifi_just_reconnect(&WiFi);
			TIME_STOP(ts_recon, "just_reconnect");
			wifi_working=recon_ok;
			DEBUG_OUT(recon_ok?"<wifi_reconnect=true>":"<wifi_reconnect=false>");
		}
		#endif

		if (!recon_ok) {
			TIME_START(ts_slow_3);
			bool try_slow = wifi_slow_connect(&WiFi);
			TIME_STOP(ts_slow_3, "try_slow_connect");
			wifi_working = try_slow;
			if (wifi_settings.force_slow) { 
				wifi_settings.force_slow=0; save_wifi_settings=true; 
			}
		}
		if (!data_ok) save_wifi_settings=true;
	#endif
	DEBUG_OUT(wifi_working?"<wifi_ok=true>":"<wifi_ok=false>");

	if (wifi_working) {
		DEBUG_OUTS("<channel="); DEBUG_OUTS(WiFi.channel()); DEBUG_OUT(">");
		DEBUG_OUTS("<bssid="); DEBUG_OUTS(WiFi.BSSIDstr().c_str()); DEBUG_OUT(">");
	}

	if (random(100)>90) {
		wifi_settings.force_slow=1;
		save_wifi_settings = true;
		DEBUG_OUT("Wifi forced next run");
	}

	if (wifi_working && save_wifi_settings) {
		DEBUG_OUT("Save settings to struct");
		TIME_START(ts_save_to_struct);
		build_settings_from_wifi(&wifi_settings, &WiFi);
		TIME_STOP(ts_save_to_struct, "save_to_struct");

		DEBUG_OUT("save_settings_to_flash ");
		TIME_START(ts_save_to_flash);
		save_settings_to_flash(&wifi_settings);
		TIME_STOP(ts_save_to_flash, "save_to_flash");

		//display_settings(&wifi_settings);
	}

	TIME_STOP(ts_setup_wifi, "setup_wifi");

	DEBUG_OUT("");

	TIME_START(ts_setup_mqtt);

	bool mqtt_worked = false;
	if (wifi_working) {
		WiFiClient wclient;
		//show_connection(&WiFi);

		DEBUG_OUT("preconnect_ip ");
		TIME_START(ts_preconnect);
		bool can_precon = preconnect_ip(&wclient, wifi_settings.mqtt_host_ip, wifi_settings.mqtt_host_port);
		TIME_STOP(ts_preconnect, "preconnect_ip");

		if (can_precon) {
			DEBUG_OUT("<preconnect=true>");
			DEBUG_OUT("publish_mqtt ");
			TIME_START(ts_mqtt_pub);
			bool pub_ok = publish_mqtt(&wclient, &wifi_settings, 
				MQTT_ACTION_TOPIC, MQTT_ACTION_VALUE);
			TIME_STOP(ts_mqtt_pub, "publish_mqtt");

			if (pub_ok) {
				mqtt_worked = true;
				DEBUG_OUT("MQTT publish OK ");
			} else {
				mqtt_worked = false;
				DEBUG_OUT("MQTT publish Failed ");
			}
		} else {
			DEBUG_OUT("<preconnect=false>");
			mqtt_worked = false;
			DEBUG_OUT("MQTT preconnect failed ");
		}
	}
	TIME_STOP(ts_setup_mqtt, "setup_mqtt");
	DEBUG_OUT(mqtt_worked?"<mqtt_ok=true>":"<mqtt_ok=false>");

	TIME_STOP(ts_setup_total, "setup_total");

	#ifdef DEBUG_MODE
	Serial.println();
	Serial.print("Duration: "); 
	Serial.print((millis()-start_time_all)); 
	Serial.println(" ms");
	#endif

	DEBUG_OUT("<complete>");
}

void loop() {
	// nothing, reboot
	if (random(100)>90) {
		DEBUG_OUT("Scanning wifi");
		WiFi.mode(WIFI_STA);
		WiFi.disconnect();
		int n = WiFi.scanNetworks(false, true);

		for (int i = 0; i < n; i++) {
			DEBUG_OUTS(i + 1);
			DEBUG_OUTS(": ");
			DEBUG_OUTS(WiFi.SSID(i));// SSID
			DEBUG_OUTS(" - ");
			DEBUG_OUTS(WiFi.RSSI(i));//Signal strength in dBm  
			DEBUG_OUTS("dBm, Channel ");
			DEBUG_OUTS(WiFi.channel(i));
			DEBUG_OUTS(" @ ");
			DEBUG_OUTS(WiFi.BSSIDstr(i));
			DEBUG_OUT("");
		}
	}
	delay(500);
	DEBUG_OUT("REBOOTING NOW");
	ESP.restart();
	DEBUG_OUT("BUT WHY");
}
