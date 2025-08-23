#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>


// --- Wi-Fi ---
bool wifiInit();
String initAP();
bool wifiConnected();
String getWiFiInfo();
void wifiLoop();            // проверка и авто-переподключение
void handleDNS();           // вызывать в loop() в режиме AP для перенаправления DNS

// --- HTTP + JSON ---
bool fetchJson(const String& url, DynamicJsonDocument& doc, bool useSecure = false, const String& userAgent = "");