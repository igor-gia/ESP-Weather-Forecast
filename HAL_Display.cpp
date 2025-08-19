#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <cmath>
#include <cstring>
#include "HAL_Display.h"
#include "pins.h"
#include "icons.h"
#include <ESP_IOExpander_Library.h>
#include "Settings.h"

namespace HAL_Display {

LGFX gfx;  // глобальный объект дисплея внутри HAL

static esp_expander::CH422G* expander = nullptr;

namespace {
    const lgfx::IFont* fontTable[] = {
        &fonts::Font0,
        &fonts::Font7,
        &fonts::FreeMono9pt7b,
        &fonts::FreeMonoBold9pt7b,
        &fonts::FreeSansOblique9pt7b,
        &fonts::FreeSerifBoldItalic12pt7b,
        &fonts::FreeSansBold24pt7b,
        &fonts::FreeSans12pt7b,
        &fonts::FreeSans18pt7b,
        &fonts::FreeSansBold18pt7b        
    };

    inline const lgfx::IFont* getFont(FontId id) {
        return fontTable[static_cast<int>(id)];
    }

    inline textdatum_t toLGFXDatum(Datum d) {
        switch(d) {
            case Datum::Left:   return textdatum_t::top_left;
            case Datum::Right:  return textdatum_t::top_right;
            case Datum::Center: return textdatum_t::top_center;
        }
        return textdatum_t::top_left;
    }

    constexpr int ICON_WIDTH  = 80;
    constexpr int ICON_HEIGHT = 80;

    void ExpanderInit() {
        expander = new esp_expander::CH422G(I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO, I2C_CH422G_ADDRESS);
        expander->init();
        expander->begin();
        expander->enableAllIO_Output();

        // Стартовые состояния
        expander->digitalWrite(TP_RST, HIGH);
        expander->digitalWrite(LCD_RST, HIGH);
        expander->digitalWrite(LCD_BL, HIGH);
        expander->digitalWrite(SD_CS, LOW);
    }
}

// --- HAL функции ---
void displayInit() {
    ExpanderInit();
    delay(500);
    gfx.init();
    gfx.setRotation(0);
    gfx.fillScreen(COL_BACKGROUND);
    setBacklight(true);
}

void setBacklight(bool state) { if(expander) expander->digitalWrite(LCD_BL,state?HIGH:LOW); }
void setPinHigh(uint8_t pin) { if(expander) expander->digitalWrite(pin,HIGH); }
void setPinLow(uint8_t pin) { if(expander) expander->digitalWrite(pin,LOW); }
bool readPin(uint8_t pin) { return expander ? expander->digitalRead(pin) : false; }

void drawString(const String& text, int x, int y, FontId font, Datum datum, uint16_t fgColor, uint16_t bgColor) {
    const lgfx::IFont* fnt = getFont(font);
    gfx.setFont(fnt);
    gfx.setTextDatum(toLGFXDatum(datum));
    gfx.setTextColor(fgColor, bgColor);
    if (font == FontId::Font7) gfx.setTextSize(2);
    else gfx.setTextSize(1);
    gfx.drawString(text, x, y);
}

void printMessage(const char* msg, int x, int y) { gfx.setCursor(x,y); gfx.print(msg); }
void fillScreen(uint16_t color) { gfx.fillScreen(color); }
void drawRect(int x,int y,int w,int h,uint16_t color) { gfx.drawRect(x,y,w,h,color); }
void drawRoundRect(int x,int y,int w,int h,int radius,uint16_t color) { gfx.drawRoundRect(x,y,w,h,radius,color); }
void fillRect(int x, int y, int w, int h, uint16_t color) { gfx.fillRect(x, y, w, h, color); }
void fillRoundRect(int x, int y, int w, int h, int radius, uint16_t color) { gfx.fillRoundRect(x, y, w, h, radius, color); }
void pushImage(int x, int y, int w, int h, const uint16_t* data) { gfx.pushImage(x, y, w, h, data); }
void readRect(int x, int y, int w, int h, uint16_t* buffer) { gfx.readRect(x, y, w, h, buffer); }

void drawIconByName(const String& name,int x,int y) {
    for(size_t i=0;i<icon_count;++i){
        if(name==icon_set[i].name){
            gfx.pushImage(x,y,ICON_WIDTH,ICON_HEIGHT,icon_set[i].data);
            return;
        }
    }
    gfx.fillRect(x,y,ICON_WIDTH,ICON_HEIGHT,TFT_BLUE);
    gfx.setTextColor(TFT_WHITE,TFT_BLUE);
    gfx.setFont(&fonts::FreeSans18pt7b);
    gfx.setTextDatum(textdatum_t::middle_center);
    gfx.drawString("N/A",x+ICON_WIDTH/2,y+ICON_HEIGHT/2);
}

void showDegree(int x,int y,uint16_t fgColor,uint16_t bgColor,FontId font){
    gfx.setFont(getFont(font));
    uint16_t widthC = gfx.textWidth("C");
    uint16_t fontHeight = gfx.fontHeight();
    int diameter = fontHeight/4; if(diameter<3) diameter=2;
    int cx = x - widthC - diameter/2;
    int cy = y + diameter/2;
    gfx.fillCircle(cx,cy,diameter/2,fgColor);
    gfx.fillCircle(cx,cy,diameter/4,bgColor);
}

void drawWindArrowTriangle(int x,int y,int size,uint16_t color,float windDirDegrees){
    int cx=x+size/2,cy=y+size/2;
    float angle=fmodf(windDirDegrees+90.0f,360.0f)*3.14159265f/180.0f;
    float halfBase=size/6.0f,height=size/2.0f;
    int tipX=cx+(int)(height*cos(angle)),tipY=cy+(int)(height*sin(angle));
    int baseCenterX=cx-(int)(height*cos(angle)),baseCenterY=cy-(int)(height*sin(angle));
    float px=-sin(angle),py=cos(angle);
    int baseLeftX=baseCenterX+(int)(px*halfBase),baseLeftY=baseCenterY+(int)(py*halfBase);
    int baseRightX=baseCenterX-(int)(px*halfBase),baseRightY=baseCenterY-(int)(py*halfBase);
    gfx.fillTriangle(tipX,tipY,baseLeftX,baseLeftY,baseRightX,baseRightY,color);
    gfx.drawCircle(cx,cy,size/2+2,color);
}

void Button(int x, int y, int w, int h, const String &label, bool pressed) {
    // Цвет кнопки
    uint16_t fillColor = pressed ? COL_FORECAST_ACTIVE : COL_FORECAST_INACTIVE;
    // Цвет текста — белый на фоне кнопки
    uint16_t textColor = COL_MENU_TEXT;

    // Закругление пропорционально размеру кнопки
    int radius = min(w, h) / 6;

    // Рисуем кнопку
    fillRoundRect(x, y, w, h, radius, fillColor);
    drawRoundRect(x, y, w, h, radius, textColor);

    // Выводим текст кнопки с центровкой
    drawString(label, x + w / 2, y + h / 2 - 6 , FMB9, D_CENTER, COL_BACKGROUND, fillColor);
}


TouchPoint getTouchDebounced(){
    static uint32_t lastTouchTime=0;
    const uint32_t debounce=200;
    TouchPoint tp={0,0,false};
    int32_t x,y;
    if(gfx.getTouch(&x,&y)){
        uint32_t now=millis();
        static int32_t lastX=-1,lastY=-1;
        if(lastX!=x||lastY!=y){lastX=x;lastY=y; return tp;}
        if(now-lastTouchTime>debounce){lastTouchTime=now; tp={x,y,true};}
    }
    return tp;
}

} // namespace HAL_Display

// --- MenuManager Impl ---
class MenuManager::Impl {
public:
    Impl(LovyanGFX& g, int w, int h, int mh)
        : gfx(g), menuSprite(&g), screenW(w), screenH(h), menuH(mh) {}

    LovyanGFX& gfx;
    LGFX_Sprite menuSprite;
    int screenW, screenH, menuH;
};

// Конструктор
MenuManager::MenuManager(int screenWidth, int screenHeight, int menuHeight)
    : screenW(screenWidth), screenH(screenHeight), menuH(menuHeight), menuYpos(screenHeight) {
    impl = new Impl(gfx, screenWidth, screenHeight, menuHeight);
}

bool MenuManager::begin() {
    size_t backupSize = screenW * menuH * sizeof(uint16_t);
    backupBuf = (uint16_t*)ps_malloc(backupSize);
    if (!backupBuf) {
        Serial.println("ERROR: can't allocate PSRAM for menu backup");
        return false;
    }
    lineBuf = new uint16_t[screenW];
    createMenuSprite();
    return true;
}

void MenuManager::createMenuSprite() {
    impl->menuSprite.setPsram(true);
    if (!impl->menuSprite.createSprite(screenW, menuH)) {
        Serial.println("ERROR: failed to create menu sprite");
        return;
    }
    updateMenuContent();
}

void MenuManager::updateMenuContent() {
    impl->menuSprite.fillSprite(TFT_BLACK);
    impl->menuSprite.fillRoundRect(1, 1, screenW - 2, menuH - 2, 15, 0x2104);
    impl->menuSprite.setTextColor(0xFFFF, 0x2104);
    impl->menuSprite.setFont(&fonts::FreeMonoBold9pt7b);
    impl->menuSprite.drawString("Information and status", 20, 20);
    impl->menuSprite.setFont(&fonts::FreeMono9pt7b);
    for (int i = 0; i < MENU_LINES; i++) {
        int y = 45 + i * 20;
        impl->menuSprite.drawString(menuLines[i], 20, y);
    }
}

void MenuManager::setMenuLine(int index, const String& text) {
    if (index < 0 || index >= MENU_LINES) return;
    menuLines[index] = text;
    updateMenuContent();
    if (menuVisible && !menuAnimating) {
        impl->menuSprite.pushSprite(0, menuYpos);
    }
}

bool MenuManager::createBackup() {
    if (!backupBuf) return false;
    impl->gfx.readRect(0, screenH - menuH, screenW, menuH, backupBuf);
    return true;
}

void MenuManager::drawPartialBackup(int yFromBottom, int lines) {
    if (yFromBottom < 0) yFromBottom = 0;
    if (yFromBottom + lines > menuH) lines = menuH - yFromBottom;

    int yScreen = screenH - menuH + yFromBottom;
    for (int i = 0; i < lines; i++) {
        memcpy(lineBuf, backupBuf + (yFromBottom + i) * screenW, screenW * sizeof(uint16_t));
        impl->gfx.pushImage(0, yScreen + i, screenW, 1, lineBuf);
    }
}

void MenuManager::show() {
    if (menuAnimating) return;
    if (!createBackup()) {
        Serial.println("Failed to backup screen before showing menu");
        return;
    }
    menuAnimating = true;
    menuVisible = false;
    menuYpos = screenH;
}

void MenuManager::hide() {
    if (menuAnimating) return;
    menuAnimating = true;
    menuVisible = true;
}

void MenuManager::update() {
    if (!menuAnimating) return;
    if (millis() - lastMoveTime < moveIntervalMs) return;
    lastMoveTime = millis();

    if (!menuVisible) {
        menuYpos -= pixelsPerStep;
        if (menuYpos <= screenH - menuH) {
            menuYpos = screenH - menuH;
            menuAnimating = false;
            menuVisible = true;
        }
        impl->menuSprite.pushSprite(0, menuYpos);
    } else {
        int yFromBottom = menuYpos - (screenH - menuH);
        if (yFromBottom < 0) yFromBottom = 0;
        drawPartialBackup(yFromBottom, pixelsPerStep);
        menuYpos += pixelsPerStep;

        if (menuYpos >= screenH) {
            menuYpos = screenH;
            menuAnimating = false;
            menuVisible = false;
            drawPartialBackup(0, menuH);
        } else {
            impl->menuSprite.pushSprite(0, menuYpos);
        }
    }
}