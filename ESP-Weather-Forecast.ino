#include "HAL_Display.h"
#include "Settings.h"
#include "HAL_Net.h"
#include "DateTime.h"
#include "weatherdata.h"
#include "Utils.h"
#include "UI.h"
#include "APWebServer.h"
#include "AirQualityData.h"

MenuManager menu(800, 480, MenuManager::MENU_LINES * 20 + 68);

bool forecastType = true;         // true - почасовый прогноз / false - ежедневный
bool pendingForecastUpdate;       // ожидание обновления прогноза (чтоб не мешать меню)
bool backLight = true;            // управление подсветкой экрана
unsigned int nightStartMinutes;
unsigned int nightEndMinutes;

unsigned long previousMillisClock;
unsigned long previousMillislWeather = 0;
unsigned long previousMillislAQ = 0;

void setup() {
  Serial.begin(115200);
  delay(500);
  loadSettings();
  Serial.println("Loaded values from NVS.");
  
  nightStartMinutes = timeToMinutes(nightStart);
  nightEndMinutes   = timeToMinutes(nightEnd);

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

  getWeatherData();
  pendingForecastUpdate = true; 
  drawString("Getting weather data from (Met.no).", 10, 150, F0, D_LEFT, COL_TEMP_NOW);

  station_uid = getNearestStation(latitude, longitude, distance);
  if(station_uid != 0) updateStationData(station_uid);
  drawString("Found nearest air quality station: " + station_name, 10, 170, F0, D_LEFT, COL_TEMP_NOW);

  String CPUi="CPU freq: "+  String(ESP.getCpuFreqMHz()) + " MHz";
  String Memi = "PSRAM: " + String((float)ESP.getPsramSize()/1024/1024, 1) + "/" + String((float)ESP.getFreePsram()/1024/1024, 1) + "MB, RAM: " + String((float)ESP.getHeapSize()/1024, 2)  + "/" + String((float)ESP.getFreeHeap()/1024, 2)  + "kB";

  drawString(CPUi, 10, 210, F0, D_LEFT, COL_TEMP_NOW);
  drawString(Memi, 10, 230, F0, D_LEFT, COL_TEMP_NOW);
  
  menu.setMenuLine(0, "Uptime: " + getUptimeString());
  menu.setMenuLine(1, "Wi-Fi: " + getWiFiInfo());
  menu.setMenuLine(3, statusNTP);
  menu.setMenuLine(4, "Latitude: " + String(latitude, 6) + ", Longitude: " + String(longitude, 6));
  menu.setMenuLine(5, "MET.no [Request: " + getDateDDMMYYYY() + " " + getTimeHHMM() + " / Updated: " + getDateDDMMYYYY(wd_meta_updated_at)+" " + getTimeHHMM(wd_meta_updated_at) + "]");
  menu.setMenuLine(6, "AQ station: " + station_name);
  String sleepMode;
  if (nightModeEnabled) {
    sleepMode = "Display will turn off at " + String(nightStart) + " and on at " + String(nightEnd);
  } else {
    sleepMode = "Sleep Mode: Disabled";
  }
  menu.setMenuLine(8, sleepMode);
  menu.setMenuLine(9, CPUi + ", " + Memi);
  
  drawString("Starting application.", 10, 250, F0, D_LEFT, COL_TEMP_NOW);

  delay(3000);

  createMainScreen();
  showDateTime();
  currentWeatherOutside();
  showAirQuality();
  showForecast(); 
}

void loop() {
  TouchPoint t = getTouchDebounced();
  if (t.touched) {
    if (backLight && t.x >= 0 && t.x <= 800 && t.y >= 212 && t.y <= 440) {
      if (!menu.isVisible() && !menu.isAnimating()) {
        // меняем тип прогноза только при неактивном меню
        forecastType = !forecastType;
        showForecast();
      }
    } else if (backLight && t.x >= BUTTON1_X && t.x <= BUTTON1_X + BUTTON1_W && t.y >= BUTTON1_Y && t.y <= BUTTON1_Y+BUTTON1_H) {
       if (!menu.isVisible() && !menu.isAnimating()) {
        // показать меню
        String CPUi = "CPU freq: "+  String(ESP.getCpuFreqMHz()) + " MHz";
        String Memi = "PSRAM: " + String((float)ESP.getPsramSize()/1024/1024, 1) + "/" + String((float)ESP.getFreePsram()/1024/1024, 1) + "MB, RAM: " + String((float)ESP.getHeapSize()/1024, 2)  + "/" + String((float)ESP.getFreeHeap()/1024, 2)  + "kB";
        menu.setMenuLine(0, "Uptime: " + getUptimeString());
        menu.setMenuLine(9, CPUi + ", " + Memi);
        menu.show();
       } else if (menu.isVisible() && !menu.isAnimating()) {
        // Спрятать меню
        menu.hide();
       }
    } else if (backLight && t.x >= BUTTON2_X && t.x <= BUTTON2_X + BUTTON2_W && t.y >= BUTTON2_Y && t.y <= BUTTON2_Y+BUTTON2_H) {
        backLight=!backLight;
        setBacklight(backLight);
    } else {
        backLight=true;
        setBacklight(backLight);
    }
  }

  wifiLoop();                            // проверка подключения Wi-Fi (каждые 5 сек)
  if (timeLoop()) {
    menu.setMenuLine(3, "NTP updated: " + getDateDDMMYYYY() + " " + getTimeHHMM());
  }
  
  handleAPServer();                      // Слушаем WEB-сервер  
  menu.update();                         // Обновляем анимацию меню, если она идёт

  if (isDue(previousMillisClock, intervalClock)) {           //обновляем дату и время и выводим их на экран с периодичностью intervalClock (1 сек)
    unsigned int currentTimeMinutes = timeToMinutes(getTimeHHMM().c_str());
    backLight = updateNightMode(backLight, currentTimeMinutes, nightStartMinutes, nightEndMinutes, nightModeEnabled);
    setBacklight(backLight);
    showDateTime();
  }

  if (isDue(previousMillislWeather, intervalWeather)) {      //обновляем значения погоды с сервера met.no (раз в 15 мин)
    getWeatherData();
    currentWeatherOutside();   
    pendingForecastUpdate = true;
    menu.setMenuLine(5, "MET.no [Request: " + getDateDDMMYYYY() + " " + getTimeHHMM() + " / Updated: " + getDateDDMMYYYY(wd_meta_updated_at)+" " + getTimeHHMM(wd_meta_updated_at) + "]");
  }

  if (isDue(previousMillislAQ, intervalAQ)) {      //обновляем значения качества воздуха (раз в 15 мин) 
    if(station_uid != 0) {
      updateStationData(station_uid); 
      showAirQuality();
      menu.setMenuLine(6, "Air Quality value updated: " + getDateDDMMYYYY() + " " + getTimeHHMM());
    }
  }

  if (!menu.isVisible() && !menu.isAnimating() && pendingForecastUpdate) {
      showForecast();
      pendingForecastUpdate = false;
    }
}
