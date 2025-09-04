// DateTime.h
#pragma once
#include <Arduino.h>
#include <time.h>

extern struct tm timeinfo;

inline double degToRad(double deg) { return deg * PI / 180.0; }
inline double radToDeg(double rad) { return rad * 180.0 / PI; }

bool checkNTP(unsigned long timeoutMs = 5000);
String getUptimeString();
void syncTime();
bool timeLoop();
int getCurrentTimeOffsetSec();
time_t parseIsoToEpoch(const String& isoUtc);
unsigned int timeToMinutes(const char* hhmm);
String getTimeHHMM(const String& isoDate = "");
String getDateDDMMYYYY(const String& isoDate = "");
String getWeekdayStr(const String& isoDate = "", bool shortName = false);
inline int dayOfYear(int y, int m, int d);
double calcSunEventUTC(int year, int month, int day, double latitude, double longitude, bool sunrise);
String formatTime(double hours, double offset);
String calculateSunrise(int y,int m,int d,double lat,double lon,int offset);
String calculateSunset(int y,int m,int d,double lat,double lon,int offset);
