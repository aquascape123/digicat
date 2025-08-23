// User_Setup.h per DigiCat - DISPLAY 2.8 POLLICI
// Adattamento da CYD-R 2.4" (ST7789) a display generici 2.8" (ILI9341)
#define USER_SETUP_INFO "DigiCat ESP32 + ILI9341 2.8inch + XPT2046 Resistive Touch"

// ======================================================
// === Driver TFT (ILI9341 per display 2.8") ==========
// ======================================================
#define ILI9341_2_DRIVER     // Driver alternativo ILI9341 per display 2.8"
// #define ILI9341_DRIVER    // Prova questo se ILI9341_2_DRIVER non funziona
// #define ST7789_DRIVER     // NON usare per display 2.8" (era per CYD-R 2.4")

// Dimensioni pannello (identiche a CYD-R)
#define TFT_WIDTH  240       
#define TFT_HEIGHT 320

// Ordine colori - da testare con il tuo display 2.8"
#define TFT_RGB_ORDER TFT_BGR  // Blue-Green-Red order (come CYD-R)
// Se i colori sono sbagliati, prova:
// #define TFT_RGB_ORDER TFT_RGB  

// Inversione display
#define TFT_INVERSION_OFF      // Come CYD-R originale
// Se il display è capovolto, prova:
// #define TFT_INVERSION_ON

// ======================================================
// === Pin ESP32 per Display 2.8" ======================
// ======================================================
#define ESP32_DMA            // Abilita DMA per performance

// CONFIGURAZIONE A - Pin basati su Bongo Cat (più comune per 2.8")
#define TFT_MISO 12          // Uguale a CYD-R
#define TFT_MOSI 13          // Uguale a CYD-R  
#define TFT_SCLK 14          // Uguale a CYD-R
#define TFT_CS   15          // Uguale a CYD-R
#define TFT_DC   2           // Uguale a CYD-R
#define TFT_RST  -1          // Uguale a CYD-R (non collegato)
#define TFT_BL   21          // CAMBIATO: era 27 per CYD-R, 21 per display 2.8"

// CONFIGURAZIONE ALTERNATIVA B - Se la A non funziona
// Decommenta queste e commenta la configurazione A
/*
#define TFT_MISO 19
#define TFT_MOSI 23  
#define TFT_SCLK 18
#define TFT_CS   5   
#define TFT_DC   2   
#define TFT_RST  4   
#define TFT_BL   22  
*/

// CONFIGURAZIONE ALTERNATIVA C - Per alcuni moduli 2.8"
/*
#define TFT_MISO -1          // Non collegato
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5    
#define TFT_DC   16   
#define TFT_RST  17   
#define TFT_BL   22   
*/

#define TFT_BACKLIGHT_ON HIGH

// Touch resistivo XPT2046 - Pin invariati da CYD-R
#define TOUCH_CS 33    // Chip Select Touch (stesso di CYD-R)
// IRQ su pin 36 (definito nel codice principale)

// ======================================================
// === SPI Settings =====================================
// ======================================================
#define USE_HSPI_PORT        // Usa HSPI per compatibilità (come CYD-R)

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
// Frequenze più conservative per display 2.8" (basate su Bongo Cat)
#define SPI_FREQUENCY        65000000   // 65MHz per display (era 80MHz per CYD-R)
#define SPI_READ_FREQUENCY   80000000   // 80MHz per lettura
#define SPI_TOUCH_FREQUENCY   2500000   // 2.5MHz per touch (invariato)

// Support SPI transactions
#define SUPPORT_TRANSACTIONS
