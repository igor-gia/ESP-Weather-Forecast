#pragma once
#include <Arduino.h>

bool isDue(unsigned long &previousMillis, unsigned long interval);
float calculateFeels(float temperatureC, float humidity, float windSpeedMS);               // вычисление ощущаемой температуры
