#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <string>

namespace HAL_Display {

// --- ENUM для шрифтов ---
enum class FontId {
    Font0,
    Font7,
    FreeMono9,
    FreeMonoBold9,
    FreeSansOblique9,
    FreeSerifBoldItalic12,
    FreeSansBold24,
    FreeSans12,
    FreeSans18,
    FreeSansBold18
};

// --- ENUM для выравнивания текста ---
enum class Datum { Left, Right, Center };



// --- Структура касания ---
struct TouchPoint {
    int32_t x, y;
    bool touched;
};

// --- Основные функции HAL (интерфейс, без зависимостей от LGFX) ---
void displayInit();
void setBacklight(bool state);
void setPinHigh(uint8_t pin);
void setPinLow(uint8_t pin);
bool readPin(uint8_t pin);

void drawString(const String& text, int x, int y, FontId font = FontId::Font0, Datum datum = Datum::Left, uint16_t fgColor = 0xFFFF, uint16_t bgColor = 0x0000);
void drawRect(int x, int y, int w, int h, uint16_t color);
void fillScreen(uint16_t color);
void fillRect(int x, int y, int w, int h, uint16_t color);
void drawRoundRect(int x, int y, int w, int h, int radius, uint16_t color);
void fillRoundRect(int x, int y, int w, int h, int radius, uint16_t color);
void pushImage(int x, int y, int w, int h, const uint16_t* data);
void readRect(int x, int y, int w, int h, uint16_t* buffer);

void printMessage(const char* msg, int x, int y);

void drawIconByName(const String& name, int x, int y);
void showDegree(int x, int y, uint16_t fgColor, uint16_t bgColor, FontId font = FontId::Font0);
void drawWindArrowTriangle(int x, int y, int size, uint16_t color, float windDirDegrees);
void Button(int x, int y, int w, int h, const String &label, bool pressed);                     // Отображает кнопку с координатами, размером и текстом
TouchPoint getTouchDebounced();

// --- Алиасы шрифтов ---
inline constexpr FontId F0    = FontId::Font0;
inline constexpr FontId F7    = FontId::Font7;
inline constexpr FontId FM9   = FontId::FreeMono9;
inline constexpr FontId FMB9  = FontId::FreeMonoBold9;
inline constexpr FontId FSO9  = FontId::FreeSansOblique9;
inline constexpr FontId FSB12 = FontId::FreeSerifBoldItalic12;
inline constexpr FontId FSB24 = FontId::FreeSansBold24;
inline constexpr FontId FS12  = FontId::FreeSans12;
inline constexpr FontId FS18  = FontId::FreeSans18;
inline constexpr FontId FSB18 = FontId::FreeSansBold18;

inline constexpr Datum D_LEFT   = Datum::Left;
inline constexpr Datum D_RIGHT  = Datum::Right;
inline constexpr Datum D_CENTER = Datum::Center;

} // namespace HAL_Display

// экспорт имён (оставляю как было, чтобы не ломать вызовы в твоём коде)
using namespace HAL_Display;

// --- MenuManager (теперь часть HAL) ---
class MenuManager {
public:

    static constexpr int MENU_LINES = 9;    // количество строк меню (от 0 до 8)

    MenuManager(int screenWidth, int screenHeight, int menuHeight);

    bool begin();
    void setMenuLine(int index, const String& text);
    void show();
    void hide();
    void update();
    bool isVisible() const { return menuVisible; }
    bool isAnimating() const { return menuAnimating; }

private:
    void createMenuSprite();
    void updateMenuContent();
    bool createBackup();
    void drawPartialBackup(int yFromBottom, int lines);

    int screenW, screenH, menuH;
    int menuYpos;
    bool menuVisible = false;
    bool menuAnimating = false;
    unsigned long lastMoveTime = 0;
    const unsigned long moveIntervalMs = 15;
    const int pixelsPerStep = 10;

    String menuLines[MENU_LINES];
    uint16_t* backupBuf = nullptr;
    uint16_t* lineBuf   = nullptr;

    // forward declaration (реализуется в cpp)
    class Impl;
    Impl* impl;
};