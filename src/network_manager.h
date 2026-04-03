#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H
#include "config.h"

// Constants
extern const uint8_t AP_PIN;

// Functions
void wifi_Setup();
void APmode_Check(); // เปลี่ยนชื่อให้สื่อความหมายว่าเป็นการเช็คปุ่ม
void mac_config();
void readNetworkConfig();
void setupWiFiMode();
void setupIPConfig();
void showAPClients();
void WiFiEvent(WiFiEvent_t event);
IPAddress parseIP(const char *ipStr);

#endif