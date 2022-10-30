
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "YourWifiName"
#define WIFI_AUTH "YourWifiPassword"

// data from DHCP: IP address, network mask, Gateway, DNS
#define DATA_IPADDR IPAddress(192,168,178,111) 
#define DATA_IPMASK IPAddress(255,255,255,0)
#define DATA_GATEWY IPAddress(192,168,178,1)
#define DATA_DNS1   IPAddress(192,168,178,1)

// data from your Wifi connect: Channel, BSSID
#define DATA_WIFI_CH 6
#define DATA_WIFI_BSSID {0x12, 0x23, 0x34, 0x45, 0x56, 0x67}

#endif
