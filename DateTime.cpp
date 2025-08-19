// DateTime.cpp
#include "DateTime.h"
#include "Settings.h"

struct tm timeinfo = {}; 

static unsigned long previousMillisTime = 0;
constexpr unsigned long TIME_UPDATE_INTERVAL = 6UL * 60UL * 60UL * 1000UL; // 6 часов

// --- Функция синхронизации времени ---
bool checkNTP(unsigned long timeoutMs) {
    configTime(0, 0, ntpServer);  // синхронизация UTC
    setenv("TZ", tzString, 1);    // часовой пояс + DST
    tzset();

    struct tm t;
    unsigned long start = millis();
    while (!getLocalTime(&t)) {
        if (millis() - start > timeoutMs) {
            return false; // сервер недоступен
        }
        delay(50);
    }
    timeinfo = t;  // обновляем глобальную структуру
    return true;
}




void syncTime() {
  configTime(0, 0, ntpServer);       // синхронизация UTC
  setenv("TZ", tzString, 1);         // часовой пояс + DST
  tzset();                           // применяем TZ
}


// --- Проверка необходимости обновления времени ---
bool timeLoop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisTime >= TIME_UPDATE_INTERVAL) {
        previousMillisTime = currentMillis;
        syncTime();
        return true; // время обновлено
    }
    return false;
}


// Функция для получения смещения в секундах от UTC по текущей TZ
int getCurrentTimeOffsetSec() {
  time_t now;
  time(&now);
  struct tm localTm = *localtime(&now);
  struct tm gmtTm   = *gmtime(&now);
  return (mktime(&localTm) - mktime(&gmtTm));
}


// Универсальный парсер ISO8601 (UTC) → time_t с учётом текущего TZ
time_t parseIsoToEpoch(const String& isoUtc) {
  int year, month, day, hour, min, sec;
  if (sscanf(isoUtc.c_str(), "%4d-%2d-%2dT%2d:%2d:%2dZ",
             &year, &month, &day, &hour, &min, &sec) != 6) {
    return -1; // ошибка парсинга
  }

  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon  = month - 1;
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min  = min;
  timeinfo.tm_sec  = sec;

  time_t epochUtc = mktime(&timeinfo);
  return epochUtc + getCurrentTimeOffsetSec();
}


// Получение времени HH:MM
String getTimeHHMM(const String& isoDate) {
  if (isoDate.length() == 0) {
    if (!getLocalTime(&timeinfo)) return "Invalid";
  } else {
    time_t localTime = parseIsoToEpoch(isoDate);
    if (localTime == -1) return "Invalid";
    timeinfo = *localtime(&localTime);
  }
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(buf);
}


// Получение даты DD/MM/YYYY
String getDateDDMMYYYY(const String& isoDate) {
  if (isoDate.length() == 0) {
    if (!getLocalTime(&timeinfo)) return "Invalid";
  } else {
    time_t localTime = parseIsoToEpoch(isoDate);
    if (localTime == -1) return "Invalid";
    timeinfo = *localtime(&localTime);
  }
  char buf[11];
  snprintf(buf, sizeof(buf), "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  return String(buf);
}


// Возвращает полное название дня недели isoDate = "" → текущая дата, иначе сокращенное название по ISO-строке (например "2025-08-13T12:00:00Z")
String getWeekdayStr(const String& isoDate, bool shortName) {
  if (isoDate.length() == 0) {
    if (!getLocalTime(&timeinfo)) return "Invalid";
  } else {
    time_t localTime = parseIsoToEpoch(isoDate);
    if (localTime == -1) return "Invalid";
    timeinfo = *localtime(&localTime);
  }
  char buf[16];
  strftime(buf, sizeof(buf), shortName ? "%a" : "%A", &timeinfo);
  return String(buf);
}


// День года с учётом високосного года
inline int dayOfYear(int y, int m, int d) {
    static const int daysBeforeMonth[] = {0,31,59,90,120,151,181,212,243,273,304,334};
    int doy = daysBeforeMonth[m-1] + d;
    if (m > 2 && ((y%4==0 && y%100!=0) || (y%400==0))) doy++;
    return doy;
}


// --- Основная функция расчёта UTC-времени восхода/заката ---
double calcSunEventUTC(int year, int month, int day, double latitude, double longitude, bool sunrise) {
    const double zenith = 90.833; // гражданский зенит
    double N = dayOfYear(year, month, day);
    double lngHour = longitude / 15.0;

    double t = sunrise ? N + ((6 - lngHour)/24) : N + ((18 - lngHour)/24);

    double M = (0.9856 * t) - 3.289;
    double L = fmod(M + 1.916*sin(degToRad(M)) + 0.020*sin(2*degToRad(M)) + 282.634, 360.0);

    double RA = radToDeg(atan2(0.91764 * tan(degToRad(L)), 1.0));
    RA = fmod(RA + 360, 360);

    double Lquadrant  = floor(L/90)*90;
    double RAquadrant = floor(RA/90)*90;
    RA += (Lquadrant - RAquadrant);
    RA /= 15.0; // в часах

    double sinDec = 0.39782 * sin(degToRad(L));
    double cosDec = cos(asin(sinDec));

    double cosH = (cos(degToRad(zenith)) - sinDec*sin(degToRad(latitude))) / (cosDec*cos(degToRad(latitude)));
    if (cosH >  1 || cosH < -1) return NAN; // полярная ночь/день

    double H = sunrise ? 360 - radToDeg(acos(cosH)) : radToDeg(acos(cosH));
    H /= 15.0;

    double T_UTC = H + RA - 0.06571*t - 6.622;
    return fmod(T_UTC - lngHour + 24, 24);
}


// --- Форматирование времени в строку "HH:MM" с учётом смещения для расчета времени рассвета и заката ---
String formatTime(double hours, double offset) {
    if (isnan(hours)) return "--:--";

    double localHours = hours + offset;          // UTC + offset
    int h = int(floor(localHours)) % 24;        // целые часы
    int m = int((localHours - floor(localHours)) * 60 + 0.5);  // минуты

    if (m >= 60) { m -= 60; h = (h + 1) % 24; }

    char buf[6];
    sprintf(buf, "%02d:%02d", h, m);
    return String(buf);
}


// --- Интерфейсные функции ---
String calculateSunrise(int y,int m,int d,double lat,double lon,int offset) {
    return formatTime(calcSunEventUTC(y,m,d,lat,lon,true), offset);
}


String calculateSunset(int y,int m,int d,double lat,double lon,int offset) {
    return formatTime(calcSunEventUTC(y,m,d,lat,lon,false), offset);
}