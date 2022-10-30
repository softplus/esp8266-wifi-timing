# ESP01 / ESP8266 wifi speed testing code

Check various ESP8266 / Arduino-framework wifi connection strategies and their timings.
What's the fastest way to connect? Where are weird things happening in the background?
We'll see.

## TLDR: Fastest ESP8266 Wifi connection

The fastest connection is done with the following setup:

1. Connect normally. Cache received BSSID & channel. O(4'000ms)
2. Set wifi.persistent(true). Connect with BSSID & channel. O(1'500ms)
3. Set wifi.persistent(true) to get persisted infos. Connect again with BSSID & channel. O(170ms) 

In C++, steps 2 & 3 look like this:

```c++
WiFi.persistent(true);
WiFi.mode(WIFI_STA);
WiFi.config(IPAddress(data->ip_address),
    IPAddress(data->ip_gateway), IPAddress(data->ip_mask), 
    IPAddress(data->ip_dns1), IPAddress(data->ip_dns2));
WiFi.begin(data->wifi_ssid, data->wifi_auth, data->wifi_channel, data->wifi_bssid, true);
uint32_t timeout = millis() + FAST_TIMEOUT;
while ((WiFi.status() != WL_CONNECTED) && (millis()<timeout)) { delay(10); }
// connected or timed out
```

## Arduino sketches

This repo includes Arduino sketches if you prefer those. They're at:

* [hardcoded connection](arduino_sketches/test_wifi_speed_hardcoded/)
* [normal connection](arduino_sketches/test_wifi_speed_flash/)

### Arduino output & explanation

```
17:13:42.923 -> Starting.
17:13:46.905 -> Timing connect(): 4001ms
17:13:46.905 -> Result=1
17:13:47.934 -> Rebooting.
17:13:48.034 -> ...
17:13:49.595 -> Starting.
17:13:50.856 -> Timing connect(): 1259ms
17:13:50.856 -> Result=1
17:13:51.851 -> Rebooting.
17:13:51.951 -> ...
17:13:53.544 -> Starting.
17:13:53.709 -> Timing connect(): 168ms
17:13:53.709 -> Result=1
17:13:54.705 -> Rebooting.
17:13:54.804 -> ...
17:13:56.397 -> Starting.
17:13:56.397 -> offset=2
17:13:56.563 -> Timing connect(): 167ms
17:13:56.563 -> Result=1
17:13:57.558 -> Rebooting.
17:13:57.657 -> ...
```

* First connection: does a normal connection with just SSID & Auth. Caches BSSID & channel. Timing is O(4'000ms)
* Second connection: does a faster connection with SSID, Auth, BSSID, channel. Then persists the connection details. TIming is O(1'500ms)
* Third (and subsequent) connnections: does a full-speed connection with SSID, Auth, BSSID, channel & persisted info. Timing is O(170ms)
* If BSSID or channel change, the fallback will go to the full connection after a 5000ms timeout. Total time on fallback is O(10'000ms).

## Variations tested

* [Notes of variants](variations.txt)
* [Google Sheet with times](https://docs.google.com/spreadsheets/d/12uTw4UXFPKMmaT33-qmTIPmPjjoEv63IqGaQU1Gozfs/edit?usp=sharing)

### Variations & timings (overview)

I included median and 90'th percentile ("p90") duration timing. 
90th percentile means that 90% of the runs were below this number.
Timings were measured with the `micros()` function, and tracked over a number of iterations.
The timing data was output as `<key=value>` to the serial port, aggregated with [/scripts/serial_monitor.sh] (a bash script that uses a Python-based serial port monitor, tracking the entries into a CSV file).

The total time includes:

* Restoring settings from flash
* Connecting to wifi
* Connecting to the MQTT server
* Publishing 1 MQTT topic (in last variant, 5 topics)

Some of the variants tested (see [variations.txt](variations.txt) for more):

* Normal connection (just SSID, auth): median 3898ms, p90 4267ms
* Connect with BSSID, channel (Variant "O"): median 1086ms, p90 1146ms
* With `persistent(true)`, no BSSID, channel specified (Variant "K"): median 2911ms, p90 3915ms
* With `persistent(true)`, giving BSSID, channel, manually connecting using `wifi_station_connect()` (Variant "J"): median 940ms, p90 1552ms
* With `persistent(true)`, giving BSSID, channel, manually connecting using `reconnect()` (variant "I"): median 587ms, p90 1749ms
* With `persistent(true)`, giving BSSID, channel, requesting connection in `begin()` (Variant "G"): median 197ms, p90 1201ms
* same, but doing 5x MQTT publishes instead of one (Variant "P"): median 188ms, p90 1227ms

The last variant ("P"; similar to the second-last one) is the one that's overall the fastest, with a median time from start to publishing 5 MQTT topics of 188ms (stddev: 334ms).

The MQTT portion of variant "P" (connecting to server, publishing topics) has a median time of 19ms (p90: 25ms, stddev: 290ms!). Without doing an IP/port pre-connection (variant "Q"), the total median time goes to 190ms (p90: 278ms, stddev: 257ms), so the IP pre-connection does not save much. Using the hostname instead of IP address (requiring a DNS lookup; variant "R"), the total time goes to median 202ms (p90 840ms, stddev 410ms), so caching the IP address is a good idea.

## ESP8266 Wifi debug output

Platformio has build flags to enable ESP wifi debug output.
Unfortunately the main variations all show exactly the same debug output, so you can't tell what's happening.

In platformio.ini:

```
build_flags = -DDEBUG_ESP_WIFI -DDEBUG_ESP_PORT=Serial 
```

Output with normal connection (scan for BSSID, channel, connect:

```
fpm close 3 
mode : sta(c4:5b:be:4a:69:34)
add if0
wifi evt: 8
wifi evt: 2
scandone
state: 0 -> 2 (b0)
state: 2 -> 3 (0)
state: 3 -> 5 (10)
add 0
aid 3
cnt 

connected with XXXXXX, channel 11
dhcp client start...
wifi evt: 0
ip:192.168.178.111,mask:255.255.255.0,gw:192.168.178.1
wifi evt: 3
<time=3791898>
```

Output without `persistent(true)` just with BSSID, Channel (variation "O"):

```
fpm close 3 
mode : sta(c4:5b:be:4a:69:34)
add if0
wifi evt: 8
wifi evt: 2
scandone
state: 0 -> 2 (b0)
state: 2 -> 3 (0)
state: 3 -> 5 (10)
add 0
aid 3
cnt 

connected with XXXXXX, channel 11
ip:192.168.178.111,mask:255.255.255.0,gw:192.168.178.1
ip:192.168.178.111,mask:255.255.255.0,gw:192.168.178.1
wifi evt: 0
wifi evt: 3
<time=1054361>
```

Output with `persistent(true)` + BSSID, channel (variation "G")

```
fpm close 3 
mode : sta(c4:5b:be:4a:69:34)
add if0
wifi evt: 8
wifi evt: 2
scandone
state: 0 -> 2 (b0)
state: 2 -> 3 (0)
state: 3 -> 5 (10)
add 0
aid 3
cnt 

connected with XXXXXX, channel 11
ip:192.168.178.111,mask:255.255.255.0,gw:192.168.178.1
ip:192.168.178.111,mask:255.255.255.0,gw:192.168.178.1
wifi evt: 0
wifi evt: 3
<time=168166>
```
