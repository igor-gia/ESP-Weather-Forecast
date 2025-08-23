#include "AirQualityData.h"
#include "Settings.h"
#include "HAL_Net.h"         // <- здесь используем fetchJson()
#include <ArduinoJson.h>
#include <cmath>

// Глобальные переменные
int aqi     = -1;
float pm1   = -1;
float pm10  = -1;
float pm25  = -1;
int station_uid = 0;
String station_name = "";
String station_address = "";

// Константы
constexpr size_t JSON_BUF_SIZE_LARGE = 16384; // для /v2/map/bounds
constexpr size_t JSON_BUF_SIZE_SMALL = 8192;  // для /feed
constexpr float DEG_LAT_M = 111320.0f;        // 1° широты ≈ 111.32 км

int getNearestStation(float lat, float lon, uint16_t radius_m) {
    float lat_delta = radius_m / 2.0f / DEG_LAT_M;
    float lon_delta = radius_m / 2.0f / (DEG_LAT_M * cos(lat * PI / 180.0f));

    float lat_min = lat - lat_delta;
    float lat_max = lat + lat_delta;
    float lon_min = lon - lon_delta;
    float lon_max = lon + lon_delta;

    String url = String(WAQI_URL) + "/v2/map/bounds?latlng=" +
                 String(lat_min, 6) + "," + String(lon_min, 6) + "," +
                 String(lat_max, 6) + "," + String(lon_max, 6) +
                 "&networks=all&token=" + WAQI_token;

    DynamicJsonDocument doc(JSON_BUF_SIZE_LARGE);
    if (!fetchJson(url, doc)) return -1;
    if (String(doc["status"]) != "ok") return -1;

    int nearest_uid = -1;
    float nearest_dist = 1e9f;
    float cos_lat = cos(lat * PI / 180.0f);

    for (JsonObject s : doc["data"].as<JsonArray>()) {
        float s_lat = s["lat"] | 0.0f;
        float s_lon = s["lon"] | 0.0f;
        int uid     = s["uid"] | -1;

        float dx = (s_lat - lat) * DEG_LAT_M;
        float dy = (s_lon - lon) * DEG_LAT_M * cos_lat;
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_uid = uid;
        }
    }

    return nearest_uid;
}

bool updateStationData(int uid) {
    if (uid == 0) return false;

    String url = String(WAQI_URL) + "/feed/@" + String(uid) + "/?token=" + WAQI_token;
    DynamicJsonDocument doc(JSON_BUF_SIZE_SMALL);

    if (!fetchJson(url, doc)) return false;
    if (String(doc["status"]) != "ok") return false;

    JsonObject data = doc["data"];

    station_uid     = uid;
    aqi             = data["aqi"] | -1;
    pm1             = data["iaqi"]["pm1"]["v"] | -1.0f;
    pm10            = data["iaqi"]["pm10"]["v"] | -1.0f;
    pm25            = data["iaqi"]["pm25"]["v"] | -1.0f;
    station_name    = String((const char*)data["city"]["name"]);
    station_address = String((const char*)data["city"]["location"]);

    return true;
}
