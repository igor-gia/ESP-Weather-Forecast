#include "APWebServer.h"
#include "Settings.h"
#include "APWebServer_HTML.h"
#include <WebServer.h>

static WebServer server(80);

// ВРЕМЕННО!!! IP точки доступа (по умолчанию 192.168.4.1)
IPAddress apIP(192, 168, 4, 1);

// Вспомогательная функция проверки строки на IP-адрес
bool isIp(const String& str) {
    for (size_t i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if ((c != '.') && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}


void handleAPServer() {
    server.handleClient();
}

void stopAPServer() {
    server.stop();
}


void handleRoot() {
  String page = FPSTR(htmlPage);
  String disabledStr = nightModeEnabled ? "" : "disabled";
  page.replace("%SSID%", ssid);
  page.replace("%PASSWORD%", password);
  page.replace("%TZ%", tzString);
  page.replace("%NTP%", ntpServer);
  page.replace("%LAT%", String(latitude, 6));
  page.replace("%LON%", String(longitude, 6));
  unsigned long intervalMinutes = intervalWeather / 1000 / 60;  // переводим мс в минуты
  page.replace("%INTERVAL%", String(intervalMinutes));
  if (nightModeEnabled) page.replace("%SLEEPEN%", "checked");
    else page.replace("%SLEEPEN%", "");
  page.replace("%NSTART%", nightStart);
  page.replace("%NEND%", nightEnd);
  page.replace("%DISABLED%", disabledStr);
  server.send(200, "text/html", page);
}

// Обработка POST-запроса
void handleSave() {
  // Считываем значения из POST
  strncpy(ssid, server.arg("ssid").c_str(), sizeof(ssid));
  ssid[sizeof(ssid)-1] = '\0';  // гарантируем null-терминатор

  strncpy(password, server.arg("password").c_str(), sizeof(password));
  password[sizeof(password)-1] = '\0';

  strncpy(tzString, server.arg("tzString").c_str(), sizeof(tzString));
  tzString[sizeof(tzString)-1] = '\0';

  strncpy(ntpServer, server.arg("ntpServer").c_str(), sizeof(ntpServer));
  ntpServer[sizeof(ntpServer)-1] = '\0';

  latitude    = server.arg("latitude").toFloat();
  longitude   = server.arg("longitude").toFloat();
  intervalWeather = server.arg("intervalWeather").toInt() * 60 * 1000UL; // минуты → мс
  
  nightModeEnabled = server.hasArg("nightModeEnabled");
  if (nightModeEnabled) {
    // Если включён, обновляем значения времени
    strncpy(nightStart, server.arg("nightStart").c_str(), sizeof(nightStart));
    nightStart[sizeof(nightStart)-1] = '\0';

    strncpy(nightEnd, server.arg("nightEnd").c_str(), sizeof(nightEnd));
    nightEnd[sizeof(nightEnd)-1] = '\0';
}

  APMode=false;
  saveSettings();

  // Отправляем информационную страницу
  String page = R"rawliteral(
<!DOCTYPE html>
<html style='font-family:sans-serif;text-align:center;padding:20px;'>
<head>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<title>Settings Saved</title>
</head>
<body>
<h2>Settings saved!</h2>
<p>ESP will reboot in 2 seconds...</p>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", page);

  // Ждём и перезагружаем ESP
  delay(3000);
  ESP.restart();
}


void handleNotFound() {
    String hostHeader = server.hostHeader();
    // Если запрос не на наш IP — перенаправляем на главную
    if (!isIp(hostHeader) && hostHeader != apIP.toString()) {
        server.sendHeader("Location", String("http://") + apIP.toString(), true);
        server.send(302, "text/plain", "");
        return;
    }
    // иначе обычный 404
    server.send(404, "text/plain", "Not Found");
}

void initAPServer() {
    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave); // POST-запрос для сохранения данных
    server.onNotFound(handleNotFound);
    server.begin();
}
