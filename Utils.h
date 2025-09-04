#pragma once
#include <Arduino.h>

bool isDue(unsigned long &previousMillis, unsigned long interval);
// вычисление ощущаемой температуры
float calculateFeels(float temperatureC, float humidity, float windSpeedMS); 
// включение и отключение экрана в зависимости от режима и нажатия кнопки
bool updateNightMode(bool backLight, unsigned int currentTimeMinutes, unsigned int nightStartMinutes, unsigned int nightEndMinutes, bool nightModeEnabled); 