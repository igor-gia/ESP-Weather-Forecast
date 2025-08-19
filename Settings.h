// Settings.h
#pragma once
#include <Arduino.h>
#include <stdint.h>


// ----------------- Constexpr: неизменяемые константы -----------------
constexpr uint16_t COL_CLOCK             = 0xFDA0;
constexpr uint16_t COL_SUNRISE           = 0x8B1E;
constexpr uint16_t COL_SUNSET            = 0xF1D1;
constexpr uint16_t COL_DATE              = 0xD6BA;
constexpr uint16_t COL_TEMP_NOW          = 0xFFFF;
constexpr uint16_t COL_TEMP_FEELS        = 0x9772;
constexpr uint16_t COL_WEATHER_INFO      = 0xA5F6;
constexpr uint16_t COL_FORECAST_ACTIVE   = 0xC618;
constexpr uint16_t COL_FORECAST_INACTIVE = 0x4208;
constexpr uint16_t COL_AUTHOR            = 0xFEA0;
constexpr uint16_t COL_DATA_PROV         = 0xAD55;
constexpr uint16_t COL_INFO              = 0x4208;
constexpr uint16_t COL_MENU_BG           = 0x2104;
constexpr uint16_t COL_MENU_TEXT         = 0xFFFF;
constexpr uint16_t COL_BACKGROUND        = 0x0000;

constexpr long intervalClock = 1000;  // 1 сек

extern bool APMode;   // режим после перезагрузки. Если true - загрузка в режиме точки доступа

// ----------------- Настраиваемые параметры -----------------
extern char ssid[32];
extern char password[32];
extern char apSSID[32];
extern char apPassword[32];
extern char tzString[64];
extern char ntpServer[32];
extern char userAgent[64];

extern float latitude;
extern float longitude;
extern long intervalNTP;
extern long intervalWeather;

extern String weatherUrl;

// ----------------- Functions -----------------
void loadSettings();
void saveSettings();
void updateAPMode(bool mode); 
void updateWeatherUrl();
