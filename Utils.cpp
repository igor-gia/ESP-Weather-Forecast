#include "Utils.h"
#include <cmath>              // для sin, cos, round

bool nightMode = false;         // ночной режим
bool lastNightMode = false;
bool autoNightMode = false;     // запоминаем предыдущее состояние
bool prevAutoNightMode = false; // Предыдущее значение autoNightMode


bool isDue(unsigned long &previousMillis, unsigned long interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}


float calculateFeels(float temperatureC, float humidity, float windSpeedMS) {
    // --- Вспомогательные функции внутри ---
    auto calcWindChill = [](float T, float v) {
        return 13.12 + 0.6215 * T - 11.37 * pow(v, 0.16) + 0.3965 * T * pow(v, 0.16);
    };
    auto calcHeatIndex = [](float T, float R) {
        return -8.784695 + 1.61139411*T + 2.338549*R
               - 0.14611605*T*R - 0.012308094*pow(T,2)
               - 0.016424828*pow(R,2) + 0.002211732*pow(T,2)*R
               + 0.00072546*T*pow(R,2) - 0.000003582*pow(T,2)*pow(R,2);
    };
    auto calcTeff = [](float T, float R, float v) {
        // Насыщенное давление водяного пара, кПа
        float e_s = 0.6108 * exp((17.27 * T) / (T + 237.3));
        // Парциальное давление, кПа
        float P = e_s * (R / 100.0);
        return -2.7 + 1.04 * T + 2.0 * P - 0.65 * v;
    };

    // --- Пороговые значения и зоны ---
    const float coldThreshold = 10.0;   // до этой температуры начинаем wind chill
    const float hotThreshold  = 27.0;   // выше этой температуры начинаем heat index
    const float blendRange    = 4.0;    // зона плавного перехода, °C

    // --- Холодная зона ---
    if (temperatureC <= coldThreshold - blendRange && windSpeedMS > 1.3) {
        return calcWindChill(temperatureC, windSpeedMS);
    }
    // --- Жаркая зона ---
    if (temperatureC >= hotThreshold + blendRange && humidity > 40.0) {
        return calcHeatIndex(temperatureC, humidity);
    }
    // --- Переход холод → умеренно ---
    if (temperatureC > coldThreshold - blendRange && temperatureC < coldThreshold && windSpeedMS > 1.3) {
        float tCold = calcWindChill(temperatureC, windSpeedMS);
        float tTeff = calcTeff(temperatureC, humidity, windSpeedMS);
        float factor = (temperatureC - (coldThreshold - blendRange)) / blendRange;
        return tCold * (1.0 - factor) + tTeff * factor;
    }
    // --- Переход умеренно → жарко ---
    if (temperatureC > hotThreshold && temperatureC < hotThreshold + blendRange && humidity > 40.0) {
        float tTeff = calcTeff(temperatureC, humidity, windSpeedMS);
        float tHot  = calcHeatIndex(temperatureC, humidity);
        float factor = (temperatureC - hotThreshold) / blendRange;
        return tTeff * (1.0 - factor) + tHot * factor;
    }
    // --- Умеренная зона ---
    return calcTeff(temperatureC, humidity, windSpeedMS);
}

// Функция переключения режимов день/ночь
bool updateNightMode(bool backLight, unsigned int currentTimeMinutes, unsigned int nightStartMinutes, unsigned int nightEndMinutes, bool nightModeEnabled) {
    static bool prevAutoNightMode = false;

    bool autoNightMode = false;
    if (nightModeEnabled) {
        if (nightStartMinutes < nightEndMinutes) {
            autoNightMode = (currentTimeMinutes >= nightStartMinutes && currentTimeMinutes < nightEndMinutes);
        } else {
            autoNightMode = (currentTimeMinutes >= nightStartMinutes || currentTimeMinutes < nightEndMinutes);
        }
    }

    if (autoNightMode != prevAutoNightMode) {
        prevAutoNightMode = autoNightMode;

        if (autoNightMode) { // ночь
            if (backLight) backLight = false;
        } else {             // день
            if (!backLight) backLight = true;
        }
    }

    return backLight;
}
