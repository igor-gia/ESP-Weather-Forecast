// Settings.cpp
#include "Settings.h"
#include <Preferences.h>

Preferences prefs;

bool APMode = false; 

// Wi-Fi
char ssid[32]     = "gia-kingdom-wi-fi";
char password[32] = "a1b2c3d4e5";

char apSSID[32]  = "ESP32_WF_AP";
char apPassword[32] = "ESP32weather";

// Часовой пояс и DST
char tzString[64] = "EET-2EEST,M3.5.0/3,M10.5.0/4";
char ntpServer[32] = "pool.time.in.ua";

// Координаты
float latitude  = 50.409133;
float longitude = 30.626686;

// User-Agent
char userAgent[64] = "MyESP32WeatherClient/1.0 gia@gia.org.ua";

// Интервал обновления
long intervalWeather = 900000;    // 15 мин

// URL погоды
String weatherUrl;

// ----------------- Functions -----------------
void updateWeatherUrl() {
    weatherUrl = "https://api.met.no/weatherapi/locationforecast/2.0/complete?lat=" + String(latitude, 6) + "&lon=" + String(longitude, 6);
}

void loadSettings() {
    prefs.begin("weather", true); // Read-only
    prefs.getString("ssid", ssid, sizeof(ssid));
    prefs.getString("password", password, sizeof(password));
    prefs.getString("tzString", tzString, sizeof(tzString));
    prefs.getString("ntpServer", ntpServer, sizeof(ntpServer));
    prefs.getString("userAgent", userAgent, sizeof(userAgent));
    latitude  = prefs.getFloat("latitude", latitude);
    longitude = prefs.getFloat("longitude", longitude);
    intervalWeather = prefs.getLong("intervalWeather", intervalWeather);
    APMode = prefs.getBool("APMode", true);
    prefs.end();
}

void saveSettings() {
    prefs.begin("weather", false); // Read/Write
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.putString("tzString", tzString);
    prefs.putString("ntpServer", ntpServer);
    prefs.putString("userAgent", userAgent);
    prefs.putFloat("latitude", latitude);
    prefs.putFloat("longitude", longitude);
    prefs.putLong("intervalWeather", intervalWeather);
    prefs.putBool("APMode", APMode); 
    prefs.end();
}

void updateAPMode(bool mode) {
    prefs.begin("weather", false);
    prefs.putBool("APMode", mode);
    prefs.end();
}