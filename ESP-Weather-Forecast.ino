#include "HAL_Display.h"
#include "Settings.h"
#include "HAL_Net.h"
#include "DateTime.h"
#include "weatherdata.h"
#include "Utils.h"
#include "UI.h"
#include "APWebServer.h"

MenuManager menu(800, 480, MenuManager::MENU_LINES * 20 + 88);

bool forecastType = true;         // true - почасовый прогноз / false - ежедневный
bool pendingForecastUpdate;       // ожидание обновления прогноза (чтоб не мешать меню)
unsigned long previousMillisClock;
unsigned long previousMillislWeather = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  loadSettings();
  Serial.println("Loaded values from NVS.");
  displayInit();
  Serial.println("Graphics initialized.");
  drawString("Booting...", 10, 10, F0, D_LEFT, COL_TEMP_NOW);
  drawString("Graphics initialized.", 10, 30, F0, D_LEFT, COL_TEMP_NOW);

  if(!APMode && strlen(ssid) != 0) {
    drawString("Starting network. Trying to connect to: " + String(ssid), 10, 50, F0, D_LEFT, COL_TEMP_NOW);
    if (!wifiInit()) {
      drawString("Wi-Fi failed!", 10, 70, F0, D_LEFT, COL_TEMP_NOW);
      APMode = true;
    } else {
      drawString("Wi-Fi OK. SSID: " + getWiFiInfo(), 10, 70, F0, D_LEFT, COL_TEMP_NOW);
      APMode = false;
    }
  }

  if(APMode || strlen(ssid) == 0) {
    Serial.println("Starting AP Mode");
    String apIP = initAP();
    initAPServer();
    drawString("Device is starting in Access Point (AP) Mode.", 10, 90, FM9, D_LEFT, COL_AUTHOR);
    drawString("Connect to this AP to change settings.", 10, 110, FM9, D_LEFT, COL_AUTHOR);
    drawString("AP name: " + String(apSSID) + ",  Password: " + String(apPassword), 10, 130, FM9, D_LEFT, COL_AUTHOR);
    drawString("Then open a browser at IP: " + apIP, 10, 150, FM9, D_LEFT, COL_AUTHOR);
    drawString("or press Reboot to restart device", 10, 170, FM9, D_LEFT, COL_AUTHOR);
    
    Button(200, 200, 100, 36, "Reboot", false);
    
    while(true) {
      handleDNS();
      handleAPServer(); 
      TouchPoint t = getTouchDebounced();
      if (t.touched) {
        if (t.x >= 200 && t.x <= 300 && t.y >= 200 && t.y <= 236) {
          Button(200, 200, 100, 36, "Reboot", true);
         
          APMode = false;
          updateAPMode(APMode);

          drawString("Rebooting...", 10, 240, F0, D_LEFT, COL_TEMP_NOW);
          delay(1000);
          ESP.restart();
        }; 
      }
    }
  }
  menu.begin();
  Serial.println("Menu initialized.");
  drawString("Menu initialized.", 10, 90, F0, D_LEFT, COL_TEMP_NOW);

  String statusNTP = "NTP unavailable";
  if (checkNTP()) {
    statusNTP = "NTP available: " + getDateDDMMYYYY() + " " + getTimeHHMM();
  }
  drawString(statusNTP, 10, 110, F0, D_LEFT, COL_TEMP_NOW );

  initAPServer();
  drawString("WEB Server initialized.", 10, 130, F0, D_LEFT, COL_TEMP_NOW);

  String statusWeather = "Weather server (Met.no) unavailable.";
  if (initWeatherClient()) {
      statusWeather = "Weather server (Met.no) available.";
  }
  drawString(statusWeather, 10, 150, F0, D_LEFT, COL_TEMP_NOW);

  getWeatherData();
  bool pendingForecastUpdate = true; 
  drawString("Getting weather data.", 10, 170, F0, D_LEFT, COL_TEMP_NOW);
  
  String CPUi="CPU freq: "+  String(ESP.getCpuFreqMHz()) + " MHz";
  String Memi = "PSRAM: " + String((float)ESP.getPsramSize()/1024/1024, 1) + "/" + String((float)ESP.getFreePsram()/1024/1024, 1) + "MB RAM: " + String((float)ESP.getHeapSize()/1024, 2)  + "/" + String((float)ESP.getFreeHeap()/1024, 2)  + "kB";

  drawString(CPUi, 10, 190, F0, D_LEFT, COL_TEMP_NOW);
  drawString(Memi, 10, 210, F0, D_LEFT, COL_TEMP_NOW);
  
  menu.setMenuLine(1, "Wi-Fi: " + getWiFiInfo());
  menu.setMenuLine(2, statusNTP);
  menu.setMenuLine(4, "Latitude: " + String(latitude, 6) + ", Longitude: " + String(longitude, 6));
  menu.setMenuLine(5, statusWeather);
  menu.setMenuLine(7, CPUi);
  menu.setMenuLine(8, Memi);
  
  drawString("Starting application.", 10, 230, F0, D_LEFT, COL_TEMP_NOW);

  delay(3000);

  createMainScreen();
  showDateTime();
  currentWeatherOutside();
  showForecast(); 
}

void loop() {
  TouchPoint t = getTouchDebounced();
  if (t.touched) {
    if (t.x >= 0 && t.x <= 800 && t.y >= 212 && t.y <= 440) {
      if (!menu.isVisible() && !menu.isAnimating()) {
        // меняем тип прогноза только при неактивном меню
        forecastType = !forecastType;
        showForecast();
      }
    } else if (t.x >= BUTTON1_X && t.x <= BUTTON1_X + BUTTON1_W && t.y >= BUTTON1_Y && t.y <= BUTTON1_Y+BUTTON1_H) {
       if (!menu.isVisible() && !menu.isAnimating()) {
        // показать меню
        String CPUi = "CPU freq: "+  String(ESP.getCpuFreqMHz()) + " MHz";
        String Memi = "PSRAM: " + String((float)ESP.getPsramSize()/1024/1024, 1) + "/" + String((float)ESP.getFreePsram()/1024/1024, 1) + "MB RAM: " + String((float)ESP.getHeapSize()/1024, 2)  + "/" + String((float)ESP.getFreeHeap()/1024, 2)  + "kB";
        menu.setMenuLine(7, CPUi);
        menu.setMenuLine(8, Memi);
        menu.show();
       } else if (menu.isVisible() && !menu.isAnimating()) {
        // Спрятать меню
        menu.hide();
       }
    } else if (t.x >= 500 && t.x <= 639 && t.y >= 441 && t.y <= 480) {
      //Serial.println(backlightState);
    }
  }
  wifiLoop();                            // проверка подключения Wi-Fi (каждые 5 сек)
  if (timeLoop()) {
    menu.setMenuLine(2, "NTP updated: " + getDateDDMMYYYY() + " " + getTimeHHMM());
  }
  
  handleAPServer();                      // Слушаем WEB-сервер  
  menu.update();                         // Обновляем анимацию меню, если она идёт

  if (isDue(previousMillisClock, intervalClock)) {           //обновляем дату и время и выводим их на экран с периодичностью intervalClock (1 сек)
    showDateTime();
  }

  if (isDue(previousMillislWeather, intervalWeather)) {      //обновляем значения погоды с сервера met.no (раз в 15 мин)
    getWeatherData();
    currentWeatherOutside();   
    pendingForecastUpdate = true;       
    menu.setMenuLine(5, "MET.no [Request: " + getDateDDMMYYYY() + " " + getTimeHHMM() + " / Updated: " + getDateDDMMYYYY(wd_meta_updated_at)+" " + getTimeHHMM(wd_meta_updated_at) + "]");
  }

  if (!menu.isVisible() && !menu.isAnimating() && pendingForecastUpdate) {
      showForecast();
      pendingForecastUpdate = false;
    }
}
