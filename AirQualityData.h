#pragma once
#include <Arduino.h>

// Функции
int getNearestStation(float lat, float lon, uint16_t radius_m);
bool updateStationData(int uid);

// Переменные
// Для выбранной станции
extern int station_uid;
extern String station_name;
extern String station_address;

// Данные AQI
extern int aqi;
extern float pm1;
extern float pm10;
extern float pm25;
