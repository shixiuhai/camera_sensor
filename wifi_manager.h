#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>

class WiFiManager {
public:
    WiFiManager();
    void begin();
    bool connectWiFi();
    void startConfigMode();

private:
    WebServer server;
    void saveWiFiConfig(const char* ssid, const char* password);
    void loadWiFiConfig(char* ssid, char* password);
    bool isConfigured();
    void handleRoot();
    void handleWiFiConfig();
    void handleStatus();
    void blinkLED(int times, int delayTime);
};

#endif // WIFI_MANAGER_H
