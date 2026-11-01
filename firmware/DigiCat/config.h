// DigiCat - build configuration
// Everything you may want to tweak lives here. No library files need editing.
#pragma once
#include <Arduino.h>

#define DIGICAT_VERSION "2.0.0"

// ---------------------------------------------------------------------------
// Board selection (can also be passed from the build: -DDIGICAT_BOARD=2)
// ---------------------------------------------------------------------------
#define BOARD_CYD_24R 1  // ESP32-2432S024R  (2.4", resistive touch on the display SPI bus)
#define BOARD_CYD_28R 2  // ESP32-2432S028R  (2.8", resistive touch on its own pins)

#ifndef DIGICAT_BOARD
#define DIGICAT_BOARD BOARD_CYD_24R
#endif

// ---------------------------------------------------------------------------
// Panel tuning. If colours look wrong (blue cat, negative image) flip these.
// ---------------------------------------------------------------------------
#if DIGICAT_BOARD == BOARD_CYD_24R
  // The 2.4" boards ship with either an ST7789 or an ILI9341 controller.
  // DigiCat v1 ran with ST7789 settings, so that's the default.
  #ifndef PANEL_IS_ILI9341
  #define PANEL_IS_ILI9341 0
  #endif
  #define PANEL_INVERT     false
  #define PANEL_RGB_ORDER  false
  #define PIN_BACKLIGHT    27
  #define SCREEN_ROTATION  2      // portrait, USB-C at the top (matches the case)
#else
  #define PANEL_IS_ILI9341 1
  #define PANEL_INVERT     false
  #define PANEL_RGB_ORDER  false
  #define PIN_BACKLIGHT    21
  #define SCREEN_ROTATION  2
#endif

// On-board RGB LED (active low on both boards). Blinks red when the cat needs you.
#define USE_RGB_LED 1
#define PIN_LED_R   4
#define PIN_LED_G   16
#define PIN_LED_B   17

// ---------------------------------------------------------------------------
// Backlight (0-255)
// ---------------------------------------------------------------------------
constexpr uint8_t  BRIGHTNESS_ACTIVE = 200;
constexpr uint8_t  BRIGHTNESS_IDLE   = 50;
constexpr uint8_t  BRIGHTNESS_NIGHT  = 12;   // lights off, cat sleeping
constexpr uint32_t IDLE_DIM_MS       = 90UL * 1000;

// ---------------------------------------------------------------------------
// Game tuning. All stats are 0..1000 internally (shown as 0..100%),
// higher is always better. Rates are per simulated minute.
// ---------------------------------------------------------------------------
#ifndef SIM_MINUTE_MS
#define SIM_MINUTE_MS 60000UL     // set to e.g. 1000 to test 60x faster
#endif

constexpr int16_t FOOD_DECAY_AWAKE   = 4;    // ~4h from full to empty
constexpr int16_t FOOD_DECAY_ASLEEP  = 2;
constexpr int16_t FUN_DECAY_AWAKE    = 5;
constexpr int16_t FUN_DECAY_ASLEEP   = 1;
constexpr int16_t ENERGY_DECAY_AWAKE = 3;
constexpr int16_t ENERGY_GAIN_ASLEEP = 20;   // ~50 min for a full night
constexpr int16_t HEALTH_LOSS        = 5;    // when any need is critical
constexpr int16_t HEALTH_GAIN        = 3;    // when all needs are fine

constexpr int16_t FEED_FOOD    = 250;
constexpr int16_t FEED_ENERGY  = 30;
constexpr int16_t PLAY_FUN     = 200;
constexpr int16_t PLAY_ENERGY  = 120;
constexpr int16_t PLAY_FOOD    = 60;
constexpr int16_t PET_FUN      = 50;
constexpr uint32_t PET_COOLDOWN_MS = 2000;

// Age (in powered-on minutes) at which the cat grows up
constexpr uint32_t AGE_YOUNG_MIN = 60;
constexpr uint32_t AGE_ADULT_MIN = 6 * 60;

// Persist to flash at most this often (plus right after reset)
constexpr uint32_t SAVE_INTERVAL_MS = 5UL * 60 * 1000;
constexpr uint32_t SAVE_AFTER_ACTION_MS = 30UL * 1000;

// Hold the title bar this long to start over
constexpr uint32_t RESET_HOLD_MS = 3000;
