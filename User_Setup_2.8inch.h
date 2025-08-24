// User_Setup.h per DigiCat - DISPLAY 2.8 POLLICI
// Basato su configurazione testata e funzionante per display 2.8" ILI9341
#define USER_SETUP_INFO "DigiCat ESP32 + ILI9341 2.8inch + XPT2046 Resistive Touch"

// ======================================================
// === Driver TFT (ILI9341 per display 2.8") ==========
// ======================================================
#define ILI9341_2_DRIVER     // Driver alternativo ILI9341 per display 2.8"

// Dimensioni pannello
#define TFT_WIDTH  240       
#define TFT_HEIGHT 320

// NON definire TFT_RGB_ORDER - lascia il default del driver
// NON definire TFT_INVERSION - lascia il default del driver

// ======================================================
// === Pin ESP32 per Display 2.8" ======================
// ======================================================

// Pin configuration testata e funzionante per 2.8"
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15          // Chip select control pin
#define TFT_DC   2           // Data Command control pin
#define TFT_RST  -1          // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST

#define TFT_BL   21          // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

// Touch resistivo XPT2046
#define TOUCH_CS 33    // Chip select pin (T_CS) of touch screen

// ======================================================
// === SPI Settings =====================================
// ======================================================
#define USE_HSPI_PORT        // Usa HSPI per compatibilità

// ======================================================
// === Font caricati ====================================
// ======================================================
#define LOAD_GLCD   
#define LOAD_FONT2  
#define LOAD_FONT4  
#define LOAD_FONT6  
#define LOAD_FONT7  
#define LOAD_FONT8  
#define LOAD_GFXFF  
#define SMOOTH_FONT

// ======================================================
// === Frequenze SPI ====================================
// ======================================================
#define SPI_FREQUENCY        55000000   // 55MHz per display (testata funzionante)
#define SPI_READ_FREQUENCY   20000000   // 20MHz per lettura
#define SPI_TOUCH_FREQUENCY   2500000   // 2.5MHz per touch

// Support SPI transactions
// #define SUPPORT_TRANSACTIONS
