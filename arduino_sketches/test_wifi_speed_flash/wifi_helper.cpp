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

#include "ESP8266WiFi.h"
#include "EEPROM.h"
#include "wifi_helper.h"

/* constructor without EEPROM offset; use 0 as offset
 */
WifiHelper::WifiHelper() {
  WifiHelper(0);
}

/* constructor with EEPROM offset
 */
WifiHelper::WifiHelper(int eeprom_offset) {
  _eeprom_offset = eeprom_offset;
}

/* Setup function to call as first step
 */
void WifiHelper::setup() {
  WiFi.setAutoConnect(false); // prevent early autoconnect
}

/* Connect to wifi as specified, returns true if ok
 */
bool WifiHelper::connect(const char *ssid, const char *auth) {
  if (_load_settings()) {
    bool res = _fast_connect();
    if (!res) res = _normal_connect();
    return res;
  }
  _init_settings(ssid, auth);
  return _normal_connect();
}

/* reset settings structure
 */
void WifiHelper::_init_settings(const char *ssid, const char *auth) {
  memset(&_settings, 0, sizeof(WIFI_SETTINGS_T));
  _settings.magic = WIFI_SETTINGS_MAGIC;
  strncpy(_settings.wifi_ssid, ssid, sizeof(_settings.wifi_ssid)-1);
  strncpy(_settings.wifi_auth, auth, sizeof(_settings.wifi_auth)-1);
}

/* load settings from EEPROM
 */
bool WifiHelper::_load_settings() {
  EEPROM.begin(256);
  EEPROM.get(_eeprom_offset, _settings);
  return (_settings.magic == WIFI_SETTINGS_MAGIC);
}

/* save settings to EEPROM
 */
bool WifiHelper::_save_settings() {
  EEPROM.begin(256);
  EEPROM.put(_eeprom_offset, _settings);
  EEPROM.end();
  return true;
}

/* do a fast-connect, if we can, return true if ok
 */
bool WifiHelper::_fast_connect() {
  if (_settings.wifi_channel==0 || _settings.wifi_bssid[0]==0) return false;
  WiFi.persistent(true); 
  WiFi.mode(WIFI_STA);
  WiFi.config(_settings.ip_address, _settings.ip_gateway, _settings.ip_mask);
  WiFi.begin(_settings.wifi_ssid, _settings.wifi_auth, _settings.wifi_channel, _settings.wifi_bssid, true);
  uint32_t timeout = millis() + 5000; // max 5s 
  while ((WiFi.status() != WL_CONNECTED) && (millis()<timeout)) { delay(5); }
  return (WiFi.status() == WL_CONNECTED);  
}

/* do a normal wifi connection, cache connection info in flash, return true if ok
 */
bool WifiHelper::_normal_connect() {
  WiFi.persistent(true); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(_settings.wifi_ssid, _settings.wifi_auth, 0, NULL, true);
  uint32_t timeout = millis() + 15000; // max 15s 
  while ((WiFi.status() != WL_CONNECTED) && (millis()<timeout)) { delay(5); }
  if (WiFi.status() == WL_CONNECTED) { // save wifi settings
    _settings.ip_address = WiFi.localIP();
    _settings.ip_gateway = WiFi.gatewayIP();
    _settings.ip_mask = WiFi.subnetMask();
    _settings.ip_dns1 = WiFi.dnsIP(0);
    _settings.ip_dns2 = WiFi.dnsIP(1);
    memcpy(_settings.wifi_bssid, WiFi.BSSID(), 6);
    _settings.wifi_channel = WiFi.channel();
    _save_settings();
    return true;
  }
  return false;
}
