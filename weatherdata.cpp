#include "weatherdata.h"
#include "Settings.h"
#include <WiFiClientSecure.h>   
#include <HTTPClient.h>        
#include <ArduinoJson.h>   // для разбора JSON

// Время последнего обновления данных на сервере (properties.meta.updated_at)
String wd_meta_updated_at = "";    

// Время, к которому относится текущий прогноз (properties.timeseries[0].time)
String wd_current_forecast_time = "";  

// Текущие данные
String wd_current_symbol_code = "";    
float  wd_current_temperature = NAN;    
float  wd_current_humidity = NAN;       
float  wd_current_pressure = NAN;       
float  wd_current_cloud_area_fraction = NAN; 
float  wd_current_uv_index = NAN;       
float  wd_current_precipitation = NAN;  
float  wd_current_wind_direction = NAN; 
float  wd_current_wind_speed = NAN;     

// Ежечасной прогноз (8 элементов)
String hourly_symbol_code[8] = { "", "", "", "", "", "", "", "" };
float  hourly_temperature[8] = { NAN, NAN, NAN, NAN, NAN, NAN, NAN, NAN };
String hourly_forecast_time[8] = { "", "", "", "", "", "", "", "" };

// Ежедневный прогноз (8 элементов)
String daily_symbol_code[8] = { "", "", "", "", "", "", "", "" };
float  daily_temperature[8] = { NAN, NAN, NAN, NAN, NAN, NAN, NAN, NAN };
String daily_forecast_time[8] = { "", "", "", "", "", "", "", "" };

WiFiClientSecure client;
HTTPClient https;

bool initWeatherClient() {
    client.setInsecure();    
    updateWeatherUrl();      
    if (!https.begin(client, weatherUrl)) {
        Serial.println("Weather client init failed");
        return false;
    }
    https.addHeader("User-Agent", userAgent);
    return true;
}

void getWeatherData() {
  int httpCode = https.GET();
  if (httpCode <= 0) {
    Serial.printf("Ошибка подключения: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    return;
  }

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("HTTP ошибка: %d\n", httpCode);
    https.end();
    return;
  }

  String payload = https.getString();
  https.end();

  // Для ускорения разбора и экономии памяти используем фильтр JSON
  const size_t capacity = 80 * 1024; // 80 KB, с запасом
  DynamicJsonDocument doc(capacity);

  // Парсим JSON
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Ошибка разбора JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // --- Сохраняем время обновления данных ---
  if (doc.containsKey("properties") && doc["properties"].containsKey("meta") && doc["properties"]["meta"].containsKey("updated_at")) {
    wd_meta_updated_at = String((const char*)doc["properties"]["meta"]["updated_at"]);
  } else {
    wd_meta_updated_at = "";
  }

  JsonArray timeseries = doc["properties"]["timeseries"].as<JsonArray>();

  if (timeseries.size() == 0) {
    Serial.println("Пустой timeseries");
    return;
  }

  // --- Текущие данные (timeseries[0]) ---
  JsonObject t0 = timeseries[0];
  wd_current_forecast_time = String((const char*)t0["time"]);

  JsonObject data0 = t0["data"];

  // Имя иконки текущей погоды (next_1_hours.summary.symbol_code)
  if (data0.containsKey("next_1_hours") && data0["next_1_hours"].containsKey("summary") && data0["next_1_hours"]["summary"].containsKey("symbol_code")) {
    wd_current_symbol_code = String((const char*)data0["next_1_hours"]["summary"]["symbol_code"]);
  } else {
    wd_current_symbol_code = "";
  }

  // instant.details
  JsonObject instant = data0["instant"]["details"];

  wd_current_temperature = instant["air_temperature"] | NAN;
  wd_current_humidity = instant["relative_humidity"] | NAN;
  wd_current_pressure = instant["air_pressure_at_sea_level"] | NAN;
  wd_current_cloud_area_fraction = instant["cloud_area_fraction"] | NAN;
  wd_current_uv_index = instant["ultraviolet_index_clear_sky"] | NAN;
  wd_current_wind_direction = instant["wind_from_direction"] | NAN;
  wd_current_wind_speed = instant["wind_speed"] | NAN;

  // Количество осадков (next_1_hours.details.precipitation_amount)
  if (data0.containsKey("next_1_hours") && data0["next_1_hours"].containsKey("details")) {
    wd_current_precipitation = data0["next_1_hours"]["details"]["precipitation_amount"] | NAN;
  } else {
    wd_current_precipitation = NAN;
  }

  // --- Ежечасной прогноз (timeseries[1..8]) ---
  for (int i = 0; i < HOURLY_FORECAST_COUNT; i++) {
    if (i + 1 >= timeseries.size()) break;

    JsonObject hourEntry = timeseries[i + 1];

    hourly_forecast_time[i] = String((const char*)hourEntry["time"]);

    JsonObject hourData = hourEntry["data"];

    // Имя иконки (next_1_hours.summary.symbol_code)
    if (hourData.containsKey("next_1_hours") && hourData["next_1_hours"].containsKey("summary") && hourData["next_1_hours"]["summary"].containsKey("symbol_code")) {
      hourly_symbol_code[i] = String((const char*)hourData["next_1_hours"]["summary"]["symbol_code"]);
    } else {
      hourly_symbol_code[i] = "";
    }

    // Температура (instant.details.air_temperature)
    if (hourData.containsKey("instant") && hourData["instant"].containsKey("details")) {
      hourly_temperature[i] = hourData["instant"]["details"]["air_temperature"] | NAN;
    } else {
      hourly_temperature[i] = NAN;
    }
  }

  // --- Ежедневный прогноз ---
  // Будем искать точки timeseries с временем около 12:00 UTC, возьмём до 8 таких точек

  int dailyCount = 0;
  for (size_t i = 0; i < timeseries.size() && dailyCount < DAILY_FORECAST_COUNT; i++) {
    String timeStr = String((const char*)timeseries[i]["time"]);
    // Пример: "2025-08-12T12:00:00Z"
    // Проверим, что время 12:00:00Z
    if (timeStr.length() >= 19 && timeStr.substring(11, 19) == "12:00:00") {
      JsonObject dayData = timeseries[i]["data"];

      daily_forecast_time[dailyCount] = timeStr;

      // Имя иконки для дневного прогноза, попробуем next_6_hours.summary.symbol_code
      if (dayData.containsKey("next_6_hours") && dayData["next_6_hours"].containsKey("summary") && dayData["next_6_hours"]["summary"].containsKey("symbol_code")) {
        daily_symbol_code[dailyCount] = String((const char*)dayData["next_6_hours"]["summary"]["symbol_code"]);
      } else {
        daily_symbol_code[dailyCount] = "";
      }

      // Температура (instant.details.air_temperature)
      if (dayData.containsKey("instant") && dayData["instant"].containsKey("details")) {
        daily_temperature[dailyCount] = dayData["instant"]["details"]["air_temperature"] | NAN;
      } else {
        daily_temperature[dailyCount] = NAN;
      }

      dailyCount++;
    }
  }

  // Если меньше 8 дней нашли — дополним пустыми
  for (; dailyCount < DAILY_FORECAST_COUNT; dailyCount++) {
    daily_forecast_time[dailyCount] = "";
    daily_symbol_code[dailyCount] = "";
    daily_temperature[dailyCount] = NAN;
  }
}

