#pragma once
#include <Arduino.h>

#define HOURLY_FORECAST_COUNT 8
#define DAILY_FORECAST_COUNT 8

// --- Функции ---
bool initWeatherClient();
void getWeatherData();  

// --- Текущие данные ---

extern String wd_meta_updated_at;                 // Время последнего обновления данных на сервере (properties.meta.updated_at)
extern String wd_current_forecast_time;           // Время, к которому относится текущий прогноз (properties.timeseries[0].time)  

// --- Текущие данные из timeseries[0].data.instant.details и next_1_hours ---

// Имя иконки текущей погоды
// Путь: properties.timeseries[0].data.next_1_hours.summary.symbol_code
extern String wd_current_symbol_code;    

// Текущая температура воздуха, °C
// Путь: properties.timeseries[0].data.instant.details.air_temperature
extern float  wd_current_temperature;    

// Текущая относительная влажность, %
// Путь: properties.timeseries[0].data.instant.details.relative_humidity
extern float  wd_current_humidity;       

// Текущее давление на уровне моря, гПа
// Путь: properties.timeseries[0].data.instant.details.air_pressure_at_sea_level
extern float  wd_current_pressure;       

// Облачность (доля неба, покрытая облаками), %
// Путь: properties.timeseries[0].data.instant.details.cloud_area_fraction
extern float  wd_current_cloud_area_fraction; 

// УФ-индекс при ясном небе (может отсутствовать)
// Путь: properties.timeseries[0].data.instant.details.ultraviolet_index_clear_sky
extern float  wd_current_uv_index;       

// Количество осадков за следующий час, мм (может отсутствовать)
// Путь: properties.timeseries[0].data.next_1_hours.details.precipitation_amount
extern float  wd_current_precipitation;  

// Направление ветра, градусы (0-360°)
// Путь: properties.timeseries[0].data.instant.details.wind_from_direction
extern float  wd_current_wind_direction; 

// Скорость ветра, м/с
// Путь: properties.timeseries[0].data.instant.details.wind_speed
extern float  wd_current_wind_speed;     

// --- Ежечасной прогноз (массив из 8 элементов) ---
// Для timeseries[1..8]

// Коллекция имен иконок для каждого часа прогноза
extern String hourly_symbol_code[8]; 

// Температуры для каждого часа прогноза
extern float  hourly_temperature[8]; 

// Время каждого часового прогноза (properties.timeseries[i].time)
extern String hourly_forecast_time[8]; 

// Коллекция имен иконок для каждого дня
extern String daily_symbol_code[8]; 

// Температуры для каждого дня (например, instant.details.air_temperature)
extern float  daily_temperature[8]; 

// Время каждого дневного прогноза (properties.timeseries[i].time)
extern String daily_forecast_time[8]; 
