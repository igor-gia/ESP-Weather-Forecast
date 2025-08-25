#include <Wire.h>
#include <LovyanGFX.hpp>
#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>

#define TFT_HOR_RES   800
#define TFT_VER_RES   480

#define TOUCH_SDA 8
#define TOUCH_SCL 9
#define TOUCH_INT 4
#define TOUCH_RST -1

// Extender Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// CH422G I2C device addresses (сдвинутые!)
#define CH422G_ADDR_WR_SET 0x24
#define CH422G_ADDR_WR_IO  0x38
#define CH422G_ADDR_RD_IO  0x26
#define CH422G_ADDR_WR_OC  0x23

// I2C Pin define
#define I2C_MASTER_NUM 0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9

//SD card Pin define
#define SD_MOSI 11    // SD card master output slave input pin
#define SD_CLK  12    // SD card clock pin
#define SD_MISO 13    // SD card master input slave output pin
#define SD_SS -1      // SD card select pin (not used)

inline void initCH422G() {
    Wire.begin(I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO); 
    // Установим все выводы как OUTPUT
    uint8_t data = 0x81;
    Wire.beginTransmission(CH422G_ADDR_WR_SET);
    Wire.write(data);
    Wire.endTransmission();
    // Все выходы HIGH
    uint8_t io_state = 0xFF;
    Wire.beginTransmission(CH422G_ADDR_WR_IO);
    Wire.write(io_state);
    Wire.endTransmission();
    delay(20);  // пауза для корректного старта
    Wire.end(); // освобождение шины
}

// ---------------------- Backlight via CH422G ----------------------
class IoExpandedBacklight : public lgfx::ILight {
public:
  void config(lgfx::ITouch::config_t *cfg) {
    if (cfg) {
      _cfg = *cfg;
    }
  }

  bool init(uint8_t brightness) override { return true; }

  void setBrightness(uint8_t brightness) override {
    uint8_t io_state = (brightness == 0) ? 0xFB : 0xFF; // если яркость 0, выключаем второй бит (11111011)
    lgfx::i2c::transactionWrite(_cfg.i2c_port, CH422G_ADDR_WR_IO, &io_state, 1, _cfg.freq);
  }

private:
  lgfx::ITouch::config_t _cfg{};
};

// -------------------------------------------------------------------

class LGFX : public lgfx::LGFX_Device {
public:
  lgfx::Bus_RGB       _bus_instance;
  lgfx::Panel_RGB     _panel_instance;
  IoExpandedBacklight _light_instance;
  lgfx::Touch_GT911   _touch_instance;

  LGFX(void) {
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width  = TFT_HOR_RES;
      cfg.memory_height = TFT_VER_RES;
      cfg.panel_width   = TFT_HOR_RES;
      cfg.panel_height  = TFT_VER_RES;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      cfg.use_psram = 1;
      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      cfg.pin_d0  = 14;
      cfg.pin_d1  = 38;
      cfg.pin_d2  = 18;
      cfg.pin_d3  = 17;
      cfg.pin_d4  = 10;

      cfg.pin_d5  = 39;
      cfg.pin_d6  = 0;
      cfg.pin_d7  = 45;
      cfg.pin_d8  = 48;
      cfg.pin_d9  = 47;
      cfg.pin_d10 = 21;

      cfg.pin_d11 = 1;
      cfg.pin_d12 = 2;
      cfg.pin_d13 = 42;
      cfg.pin_d14 = 41;
      cfg.pin_d15 = 40;

      cfg.pin_henable = 5;
      cfg.pin_vsync   = 3;
      cfg.pin_hsync   = 46;
      cfg.pin_pclk    = 7;
      cfg.freq_write  = 14000000;

      cfg.hsync_polarity     = 0;
      cfg.hsync_front_porch  = 20;
      cfg.hsync_pulse_width  = 10;
      cfg.hsync_back_porch   = 10;

      cfg.vsync_polarity     = 0;
      cfg.vsync_front_porch  = 10;
      cfg.vsync_pulse_width  = 10;
      cfg.vsync_back_porch   = 10;

      cfg.pclk_active_neg = 0;
      cfg.de_idle_high    = 0;
      cfg.pclk_idle_high  = 0;

      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      auto cfg = _touch_instance.config();
      cfg.x_min = 0;
      cfg.x_max = TFT_HOR_RES - 1;
      cfg.y_min = 0;
      cfg.y_max = TFT_VER_RES - 1;
      cfg.pin_int = TOUCH_INT;
      cfg.pin_rst = TOUCH_RST;
      cfg.bus_shared = false; 
      cfg.offset_rotation = 0;
      cfg.i2c_port = I2C_NUM_1;
      cfg.pin_sda  = TOUCH_SDA;
      cfg.pin_scl  = TOUCH_SCL;
      cfg.freq     = 400000;
      cfg.i2c_addr = 0x14;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    {
      auto touch_cfg = _touch_instance.config();
      _light_instance.config(&touch_cfg);
      _panel_instance.setLight(&_light_instance);
    }

    setPanel(&_panel_instance);
  }
};

