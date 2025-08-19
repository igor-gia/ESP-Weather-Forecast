#include <WiFi.h>
#include <DNSServer.h>
#include "HAL_Net.h"
#include "Settings.h"

static uint32_t lastWifiCheck = 0;
static const uint32_t wifiCheckInterval = 5000; // каждые 5 сек проверка Wi-Fi

DNSServer dnsServer;              
const byte DNS_PORT = 53;

// --- Wi-Fi ---
bool wifiInit() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to Wi-Fi");
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - start > 20000) {
            Serial.println(" Failed!");
            return false;
        }
    }
    Serial.println(" OK");
    return true;
}

String initAP() {
    WiFi.mode(WIFI_AP);
    if (strlen(apPassword) >= 8) {
        WiFi.softAP(apSSID, apPassword);
    } else {
        WiFi.softAP(apSSID);
    }
    IPAddress apIP = WiFi.softAPIP();
    dnsServer.start(DNS_PORT, "*", apIP);
    return apIP.toString();
}

bool wifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}


String getWiFiInfo() {
    if (!wifiConnected()) return "Wi-Fi: not connected";
    return WiFi.SSID() + " / RSSI: " + String(WiFi.RSSI()) + " dBm / IP: " + WiFi.localIP().toString();
}

// --- авто-переподключение ---
void wifiLoop() {
    if (millis() - lastWifiCheck < wifiCheckInterval) return;
    lastWifiCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi lost, reconnecting...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
    }
}

void handleDNS() {
    dnsServer.processNextRequest();   // обёртка
}
