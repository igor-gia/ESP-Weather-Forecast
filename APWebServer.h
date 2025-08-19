#pragma once
#include <WiFi.h>

void initAPServer();          // запуск веб-сервера
void handleAPServer();        // обработка клиентов
void stopAPServer();          // остановка сервера
