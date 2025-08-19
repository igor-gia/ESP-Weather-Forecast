#pragma once
#include <Arduino.h>


// --- Wi-Fi ---
bool wifiInit();
String initAP();
bool wifiConnected();
String getWiFiInfo();
void wifiLoop();            // проверка и авто-переподключение
void handleDNS();           // вызывать в loop() в режиме AP для перенаправления DNS