#include "UI.h"
#include "DateTime.h"   // для получения времени
#include "weatherdata.h" // для данных погоды
#include "HAL_Display.h" // для вывода на экран
#include "Settings.h"
#include "Utils.h"

// ------ коодинаты отображаемых элементов --------
constexpr int CLOCK_X = 795;
constexpr int CLOCK_Y = 5;
constexpr int WEEK_X = 795;
constexpr int WEEK_Y = 115;
constexpr int DATE_X = 795;
constexpr int DATE_Y = 160;
constexpr int SUNRISE_X = 605;
constexpr int SUNRISE_Y = 125;
constexpr int SUNSET_X = 605;
constexpr int SUNSET_Y = 165;
constexpr int CURTEMP_X = 215;
constexpr int CURTEMP_Y = 10;
constexpr int FLTEMP_X = 490;
constexpr int FLTEMP_Y = 10;
constexpr int HUM_X = 55;
constexpr int HUM_Y = 125;
constexpr int PRESS_X = 55;
constexpr int PRESS_Y = 165;
constexpr int CLOUDS_X = 230;
constexpr int CLOUDS_Y = 125;
constexpr int UV_X = 230;
constexpr int UV_Y = 165;
constexpr int PRESIP_X = 350;
constexpr int PRESIP_Y = 125;
constexpr int WIND_D_X = 348;
constexpr int WIND_D_Y = 160;
constexpr int WIND_S_X = 380;
constexpr int WIND_S_Y = 165;

constexpr int HEADER_FORECAST_X = 400;
constexpr int HEADER_FORECAST_Y = 228;  // уточнять!

constexpr int D_T_FORECAST_Y = 270;
constexpr int ICON_FORECAST_Y = 302;
constexpr int TEMP_FORECAST_Y = 390;

constexpr int AUTHOR_X = 5;
constexpr int AUTHOR_Y = 446;    // уточнять!
constexpr int DATA_PROV_X = 5;
constexpr int DATA_PROV_Y = 460;    // уточнять!
constexpr int INFO_X = 795;
constexpr int INFO_Y = 453;    // уточнять!

extern bool forecastType;

String prevTime = "";
String prevDate = "";

void createMainScreen() {
  fillScreen(COL_BACKGROUND);

  drawIconByName("label1", 5, 110); 
  drawIconByName("label2", 180, 110);
  drawIconByName("label3", 300, 110);
  drawIconByName("label4", 500, 110);

  drawString("Created by Igor Gimelfarb", AUTHOR_X, AUTHOR_Y, F0, D_LEFT, COL_AUTHOR, COL_BACKGROUND);
  drawString("Data provided by MET Norway (met.no)", DATA_PROV_X, DATA_PROV_Y, F0, D_LEFT, COL_DATA_PROV, COL_BACKGROUND);
  Button(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, "Info", false);
}

void showDateTime() {
  String currentTime = getTimeHHMM();
  String currentDate = getDateDDMMYYYY();

  //часы
  if (currentTime != prevTime) {
    fillRect (500, 4, 298, 105,  COL_BACKGROUND); 
    drawString(currentTime, CLOCK_X, CLOCK_Y, F7, D_RIGHT, COL_CLOCK, COL_BACKGROUND);
    prevTime = currentTime;
  }
  //дата
  if (currentDate != prevDate) {
    // отображаем новую дату
    fillRect (540, 110, 258, 90, COL_BACKGROUND); 
    drawString(getWeekdayStr(), WEEK_X, WEEK_Y, FS18, D_RIGHT, COL_DATE, COL_BACKGROUND);
    drawString(currentDate, DATE_X, DATE_Y, FS18, D_RIGHT, COL_DATE, COL_BACKGROUND);
    // вычисляем и отображаем время восхода и заката
    drawString(calculateSunrise(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, latitude, longitude, getCurrentTimeOffsetSec()/60/60), SUNRISE_X, SUNRISE_Y, FS12, D_RIGHT, COL_SUNRISE, COL_BACKGROUND);
    drawString(calculateSunset(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, latitude, longitude, getCurrentTimeOffsetSec()/60/60), SUNSET_X, SUNSET_Y, FS12, D_RIGHT, COL_SUNSET, COL_BACKGROUND);
    prevDate = currentDate;
  }
}

void currentWeatherOutside() {
  drawIconByName(wd_current_symbol_code, 10, 10);
  // температура
  fillRect (91, 4, 408, 107, COL_BACKGROUND);    // очистка экрана от старых данных
  drawString(String(wd_current_temperature, 0)+" C", CURTEMP_X, CURTEMP_Y, FSB24, D_RIGHT, COL_TEMP_NOW, COL_BACKGROUND);
  showDegree (CURTEMP_X, CURTEMP_Y, COL_TEMP_NOW, COL_BACKGROUND, FSB24);
  drawString("Feels: " + String(calculateFeels(wd_current_temperature, wd_current_humidity, wd_current_wind_speed),0) + " C", FLTEMP_X, FLTEMP_Y, FSB24, D_RIGHT, COL_TEMP_FEELS, COL_BACKGROUND);
  showDegree (FLTEMP_X, FLTEMP_Y, COL_TEMP_FEELS, COL_BACKGROUND, FSB24);

  // влажность и давление
  fillRect (52, 110, 127, 88, COL_BACKGROUND);   // очистка экрана от старых данных
  drawString(String(wd_current_humidity, 2) + "%", HUM_X, HUM_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);
  drawString(String(wd_current_pressure, 0) + " hPa", PRESS_X, PRESS_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);

  // облачность и УФ
  fillRect (227, 110, 72, 88, COL_BACKGROUND);   // очистка экрана от старых данных
  drawString(String(wd_current_cloud_area_fraction, 0) + "%", CLOUDS_X, CLOUDS_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);
  drawString(String(wd_current_uv_index), UV_X, UV_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);

    // осадки и ветер
  fillRect (345, 110, 135, 88, COL_BACKGROUND);   // очистка экрана от старых данных
  drawString(String(wd_current_precipitation) + " mm", PRESIP_X, PRESIP_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);
  drawWindArrowTriangle(WIND_D_X, WIND_D_Y, 25, COL_WEATHER_INFO, wd_current_wind_direction);      // Рисуем стрелку направления ветра
  drawString(String(wd_current_wind_speed, (wd_current_wind_speed < 10) ? 1 : 0) + " m/s", WIND_S_X, WIND_S_Y, FS12, D_LEFT, COL_WEATHER_INFO, COL_BACKGROUND);
}


void showForecast() {
  fillRect(0, 212, 800, 228, COL_BACKGROUND);   // очистка экрана от старых данных

  drawRoundRect(0, 212, 800, 228, 20, COL_FORECAST_ACTIVE);
  drawRoundRect(1, 213, 798, 226, 19, COL_FORECAST_ACTIVE);

  drawString("Hourly Forecast", HEADER_FORECAST_X - 10, HEADER_FORECAST_Y, FSB18, D_RIGHT, forecastType ? COL_FORECAST_ACTIVE : COL_FORECAST_INACTIVE, COL_BACKGROUND);
  drawString("Daily Forecast", HEADER_FORECAST_X + 10, HEADER_FORECAST_Y, FSB18, D_LEFT, forecastType ? COL_FORECAST_INACTIVE : COL_FORECAST_ACTIVE, COL_BACKGROUND);

  for (int i = 0; i < 8; i++) {
    if (forecastType) {   
      // выводим прогноз почасовый
      drawString(getTimeHHMM(hourly_forecast_time[i]), i * 100 + 88, D_T_FORECAST_Y, FS18, D_RIGHT, COL_FORECAST_ACTIVE, COL_BACKGROUND);
      drawIconByName(hourly_symbol_code[i], i * 100 + 10, ICON_FORECAST_Y);
      drawString(String(hourly_temperature[i],0) + " C", i * 100 + 88, TEMP_FORECAST_Y, FS18, D_RIGHT, COL_FORECAST_ACTIVE, COL_BACKGROUND);
      showDegree (i * 100 + 88, TEMP_FORECAST_Y, COL_FORECAST_ACTIVE, COL_BACKGROUND, FS18);
    } else {
      // выводим прогноз поденный
      drawString(getWeekdayStr(daily_forecast_time[i], true), i * 100 + 88, D_T_FORECAST_Y, FS18, D_RIGHT, COL_FORECAST_ACTIVE, COL_BACKGROUND);
      drawIconByName(daily_symbol_code[i], i * 100 + 10, ICON_FORECAST_Y);
      drawString(String(daily_temperature[i],0) + " C", i * 100 + 88, TEMP_FORECAST_Y, FS18, D_RIGHT, COL_FORECAST_ACTIVE, COL_BACKGROUND);
      showDegree (i * 100 + 88, TEMP_FORECAST_Y, COL_FORECAST_ACTIVE, COL_BACKGROUND, FS18);
    }
  }

}

