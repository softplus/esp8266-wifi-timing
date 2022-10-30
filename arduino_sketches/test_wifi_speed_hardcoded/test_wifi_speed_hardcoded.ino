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

/* Uses hardcoded wifi data to connect, as a proof-of-concept.
 *  
 * DO NOT USE THIS CODE - if you ever change your wifi access point, or if it
 * changes channels (which they tend to do), your code will fail. It will not
 * try to connect to your access point in a normal way.
 * 
 * Use the other example if you want to copy & paste code.
 */

#include "ESP8266WiFi.h"
#include "secrets.h"

/* Only do a fast connection with a given SSID, password, BSSID, channel.
 * This will fail when your access point does its normal work.
 */
int wifi_fast() {
  WiFi.persistent(true); 
  WiFi.mode(WIFI_STA);
  WiFi.config(DATA_IPADDR, DATA_GATEWY, DATA_IPMASK);
  uint8_t my_bssid[] = DATA_WIFI_BSSID;
  char my_ssid[] = WIFI_SSID;
  char my_auth[] = WIFI_AUTH;
  WiFi.begin(&my_ssid[0], &my_auth[0], DATA_WIFI_CH, &my_bssid[0], true);
  uint32_t timeout = millis() + 5000;
  while ((WiFi.status() != WL_CONNECTED) && (millis()<timeout)) { delay(5); }
  return (WiFi.status() == WL_CONNECTED);  
}

/* One-time setup function. Does wifi connection and shows timing.
 */
void setup() {
  WiFi.setAutoConnect(false); // prevent early autoconnect
  Serial.begin(115200);
  delay(1500); // just for debugging
  Serial.println("\nStarting.");
  uint32_t ts_start = millis();
  int wifi_fast_ok = wifi_fast();
  Serial.print("Timing wifi_fast(): "); Serial.print(millis()-ts_start); Serial.println("ms");
  Serial.print("Result="); Serial.println(wifi_fast_ok);
}

/* Main loop: waits 1 second, then reboots - so that it tries to connect multiple times.
 */
void loop() {
  delay(1000); // wait, reboot, try again. 
  Serial.println("Rebooting.");
  ESP.restart();
}
