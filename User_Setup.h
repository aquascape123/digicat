// User_Setup.h per CYD-R (2432S024R) - Touchscreen RESISTIVO
#define USER_SETUP_INFO "CYD-R ESP32 + ST7789 + XPT2046 Resistive Touch"

// ======================================================
// === Driver TFT (ST7789 per CYD-R) ===================
// ======================================================
#define ST7789_DRIVER        // Driver corretto per CYD-R
// #define ILI9341_DRIVER    // NON usare questo per CYD-R

// Dimensioni pannello CYD-R
#define TFT_WIDTH  240       // ATTENZIONE: diverso da CYD-C!
#define TFT_HEIGHT 320

// Ordine colori e inversione
#define TFT_RGB_ORDER TFT_BGR  // Blue-Green-Red order
#define TFT_INVERSION_OFF      // Non invertire

// ======================================================
// === Pin ESP32 per CYD-R ==============================
// ======================================================
#define ESP32_DMA            // Abilita DMA

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15   // Chip Select TFT
#define TFT_DC   2    // Data/Command
#define TFT_RST  -1   // Reset non collegato
#define TFT_BL   27   // Retroilluminazione

#define TFT_BACKLIGHT_ON HIGH

// Touch resistivo XPT2046
#define TOUCH_CS 33    // Chip Select Touch
// IRQ collegato al pin 36 (da usare nel codice, non qui)

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
#define SPI_FREQUENCY        80000000   // 80MHz per display
#define SPI_READ_FREQUENCY   80000000   // 80MHz per lettura
#define SPI_TOUCH_FREQUENCY   2500000   // 2.5MHz per touch
