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

/* Builds up a normal wifi connection, caches BSSID, channel for a faster connection,
 * which additionally persists the channel encryption data for connections in O(200ms).
 * 
 * Caches 132 bytes in EEPROM/Flash at offset 0 (can be changed)
 * 
 * Create your own secrets.h using the secrets_sample.h.
 */
#include "secrets.h"
#include "wifi_helper.h"

WifiHelper wh; // use wh(123) to set EEPROM offset

void setup() {
  wh.setup();
  Serial.begin(115200);
  delay(1500); // just for debugging
  Serial.println("\nStarting.");
  uint32_t ts_start = millis();
  bool res = wh.connect(WIFI_SSID, WIFI_AUTH);
  Serial.print("Timing connect(): "); Serial.print(millis()-ts_start); Serial.println("ms");
  Serial.print("Result="); Serial.println(res);
}

void loop() {
  delay(1000); // wait, reboot, try again. 
  Serial.println("Rebooting.");
  ESP.restart();
}
