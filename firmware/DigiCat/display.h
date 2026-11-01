// DigiCat - LovyanGFX setup for the Cheap Yellow Display boards
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "config.h"

class LGFX : public lgfx::LGFX_Device {
#if PANEL_IS_ILI9341
  lgfx::Panel_ILI9341 _panel;
#else
  lgfx::Panel_ST7789 _panel;
#endif
  lgfx::Bus_SPI       _bus;
  lgfx::Light_PWM     _light;
  lgfx::Touch_XPT2046 _touch;

 public:
  LGFX() {
    {
      auto cfg = _bus.config();
      cfg.spi_host    = SPI2_HOST;  // HSPI
      cfg.spi_mode    = 0;
      cfg.freq_write  = 40000000;
      cfg.freq_read   = 16000000;
      cfg.spi_3wire   = false;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk    = 14;
      cfg.pin_mosi    = 13;
      cfg.pin_miso    = 12;
      cfg.pin_dc      = 2;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs           = 15;
      cfg.pin_rst          = -1;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = true;
      cfg.invert           = PANEL_INVERT;
      cfg.rgb_order        = PANEL_RGB_ORDER;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = (DIGICAT_BOARD == BOARD_CYD_24R);
      _panel.config(cfg);
    }
    {
      auto cfg = _light.config();
      cfg.pin_bl      = PIN_BACKLIGHT;
      cfg.invert      = false;
      cfg.freq        = 12000;
      cfg.pwm_channel = 7;
      _light.config(cfg);
      _panel.setLight(&_light);
    }
    {
      // Raw ranges are only a starting point: DigiCat runs a 4-corner
      // calibration on first boot and stores the result in flash.
      auto cfg = _touch.config();
      cfg.x_min           = 300;
      cfg.x_max           = 3900;
      cfg.y_min           = 200;
      cfg.y_max           = 3700;
      cfg.pin_int         = 36;
      cfg.offset_rotation = 0;
      cfg.freq            = 1000000;
#if DIGICAT_BOARD == BOARD_CYD_24R
      cfg.spi_host   = SPI2_HOST;  // shares the display bus
      cfg.bus_shared = true;
      cfg.pin_sclk   = 14;
      cfg.pin_mosi   = 13;
      cfg.pin_miso   = 12;
#else
      cfg.spi_host   = SPI3_HOST;  // VSPI, dedicated touch pins
      cfg.bus_shared = false;
      cfg.pin_sclk   = 25;
      cfg.pin_mosi   = 32;
      cfg.pin_miso   = 39;
#endif
      cfg.pin_cs = 33;
      _touch.config(cfg);
      _panel.setTouch(&_touch);
    }
    setPanel(&_panel);
  }
};
