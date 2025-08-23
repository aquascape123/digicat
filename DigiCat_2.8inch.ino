/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║                           DigiCat v1.0.0                        ║
 * ║                 Virtual Pet for ESP32 + 2.8" Display            ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║                                                                  ║
 * ║  🐱 A charming virtual cat companion with pixel art animations  ║
 * ║  📱 Touch interface for feeding, playing, and petting          ║
 * ║  🏆 Evolution system from kitten to adult cat                  ║
 * ║  😴 Dynamic moods, expressions, and sleep cycles               ║
 * ║  🎮 Chill gameplay with realistic stat progression             ║
 * ║                                                                  ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  Hardware: ESP32 + 2.8" ILI9341 Display + Resistive Touch      ║
 * ║  Display:  240x320 ILI9341 TFT                                 ║
 * ║  Touch:    XPT2046 Resistive Controller                        ║
 * ║                                                                  ║
 * ║  Libraries Required:                                            ║
 * ║  - TFT_eSPI by Bodmer                                          ║
 * ║  - XPT2046_Touchscreen by Paul Stoffregen                     ║
 * ║                                                                  ║
 * ║  IMPORTANT: Use User_Setup_DigiCat_2.8inch.h for TFT_eSPI!    ║
 * ║                                                                  ║
 * ║  Made with ❤️ for the ESP32 maker community                    ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen setup for 2.8" Display (pin same as CYD-R)
#define XPT2046_IRQ 36   // T_IRQ - Touch interrupt pin
#define XPT2046_MOSI 13  // T_DIN - SPI MOSI
#define XPT2046_MISO 12  // T_OUT - SPI MISO  
#define XPT2046_CLK 14   // T_CLK - SPI Clock
#define XPT2046_CS 33    // T_CS  - Touch chip select

SPIClass touchscreenSPI = SPIClass(HSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Pet State Structure
struct Pet {
  String name;
  int happiness;
  int hunger;
  int health;
  int age;
  unsigned long lastUpdate;
  unsigned long lastFeed;
  unsigned long lastPlay;
  bool sleeping;
  int evolutionStage; // 0=kitten, 1=young cat, 2=adult cat
  bool blinking;
  unsigned long lastBlink;
  int animationFrame;
} pet;

// UI Layout for 240x320 portrait display (invariato)
const int SCREEN_WIDTH = 240;   
const int SCREEN_HEIGHT = 320;
const int PET_X = 80;           // Cat position 
const int PET_Y = 260;          

// Touch button definitions (invariate)
struct TouchButton {
  int x, y, width, height;
  String label;
  uint16_t color;
  bool pressed;
};

TouchButton buttons[3] = {
  {20, 50, 90, 45, "FEED", TFT_YELLOW, false},     // Feed button
  {130, 50, 90, 45, "PLAY", TFT_GREEN, false},     // Play button  
  {160, 270, 70, 35, "RESET", TFT_RED, false}      // Reset button (bottom right)
};

// Color definitions (invariate)
const uint16_t BG_COLOR = TFT_CYAN;
const uint16_t UI_COLOR = TFT_BLACK;
const uint16_t BAR_BG = TFT_WHITE;

// Cat colors (invariate)
const uint16_t CAT_BODY = TFT_ORANGE;
const uint16_t CAT_STRIPE = TFT_BLACK;
const uint16_t CAT_EYES = TFT_GREEN;
const uint16_t CAT_NOSE = TFT_PINK;

unsigned long lastTouch = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("╔══════════════════════════════════════════╗");
  Serial.println("║            DigiCat v1.0.0                ║");
  Serial.println("║      Virtual Pet for ESP32 2.8\" TFT     ║");
  Serial.println("╚══════════════════════════════════════════╝");
  Serial.println();
  
  // Initialize touchscreen with dedicated SPI
  Serial.println("🔧 Initializing touchscreen...");
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  Serial.println("✅ Touchscreen ready!");
  
  // Initialize display (portrait mode rotated 180°)
  Serial.println("🖥️  Initializing 2.8\" display...");
  tft.init();
  tft.setRotation(2); // Portrait rotated 180 degrees
  
  // Test per verificare driver display 2.8"
  Serial.println("🧪 Testing 2.8\" ILI9341 display...");
  tft.fillScreen(TFT_RED);
  delay(500);
  tft.fillScreen(TFT_GREEN);  
  delay(500);
  tft.fillScreen(TFT_BLUE);
  delay(500);
  Serial.println("✅ Display test complete!");
  
  // Initialize pet
  Serial.println("🐱 Initializing DigiCat...");
  initializePet();
  Serial.println("✅ DigiCat ready for 2.8\" display!");
  
  Serial.println();
  Serial.println("🎮 Controls:");
  Serial.println("   📱 Touch buttons to interact");
  Serial.println("   🍖 FEED - Reduce hunger");
  Serial.println("   🎾 PLAY - Increase happiness");
  Serial.println("   🔄 RESET - Start over");
  Serial.println("   👋 Pet cat directly for bonus happiness!");
  Serial.println();
  Serial.println("🎯 Your DigiCat adventure begins now!");
  Serial.println("═══════════════════════════════════════════");
  
  drawScreen();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Update pet stats every 30 seconds (much more chill!)
  if (currentTime - pet.lastUpdate > 30000) {
    updatePetStats();
    pet.lastUpdate = currentTime;
    drawScreen();
  }
  
  // Handle pet blinking
  if (currentTime - pet.lastBlink > random(3000, 8000)) {
    pet.blinking = !pet.blinking;
    pet.lastBlink = currentTime;
    drawCat(); // Redraw only cat for blinking
  }
  
  // Check for touch input
  handleTouch();
  
  delay(100);
}

void initializePet() {
  pet.name = "DigiCat";
  pet.happiness = 50;
  pet.hunger = 30;
  pet.health = 80;
  pet.age = 0;
  pet.lastUpdate = millis();
  pet.lastFeed = millis();
  pet.lastPlay = millis();
  pet.sleeping = false;
  pet.evolutionStage = 0; // Start as kitten
  pet.blinking = false;
  pet.lastBlink = millis();
  pet.animationFrame = 0;
  
  Serial.println("🐱 " + pet.name + " is born!");
  Serial.println("   Stage: Kitten");
  Serial.println("   Happiness: " + String(pet.happiness) + "%");
  Serial.println("   Hunger: " + String(pet.hunger) + "%");
  Serial.println("   Health: " + String(pet.health) + "%");
}

void updatePetStats() {
  Serial.println("📊 Updating pet stats...");
  
  // Hunger increases over time (every 5 minutes)
  if (millis() - pet.lastFeed > 300000) { // 5 minutes
    pet.hunger = min(100, pet.hunger + 5);
    pet.lastFeed = millis();
    Serial.println("   🍽️  Hunger increased to: " + String(pet.hunger) + "%");
  }
  
  // Happiness decreases slowly without attention
  if (millis() - pet.lastPlay > 600000) { // 10 minutes  
    pet.happiness = max(0, pet.happiness - 3);
    Serial.println("   😔 Happiness decreased to: " + String(pet.happiness) + "%");
  }
  
  // Health depends on hunger and happiness
  int targetHealth = 100;
  if (pet.hunger > 80) targetHealth -= 30;
  else if (pet.hunger > 60) targetHealth -= 15;
  
  if (pet.happiness < 20) targetHealth -= 25;
  else if (pet.happiness < 40) targetHealth -= 10;
  
  // Gradual health adjustment
  if (pet.health < targetHealth) {
    pet.health = min(100, pet.health + 5);
  } else if (pet.health > targetHealth) {
    pet.health = max(0, pet.health - 3);
  }
  
  // Check if sleeping (when health is very low)
  pet.sleeping = (pet.health < 30);
  if (pet.sleeping && !pet.sleeping) {
    Serial.println("   😴 " + pet.name + " fell asleep (low health)");
  }
  
  // Evolution system
  pet.age++;
  int oldStage = pet.evolutionStage;
  
  if (pet.age > 100 && pet.evolutionStage == 0) {
    pet.evolutionStage = 1; // Young cat
  } else if (pet.age > 300 && pet.evolutionStage == 1) {
    pet.evolutionStage = 2; // Adult cat
  }
  
  if (pet.evolutionStage != oldStage) {
    String stages[] = {"Kitten", "Young Cat", "Adult Cat"};
    Serial.println("🎉 " + pet.name + " evolved to: " + stages[pet.evolutionStage]);
    
    // Evolution celebration flash
    tft.fillScreen(TFT_YELLOW);
    delay(200);
    tft.fillScreen(TFT_MAGENTA);
    delay(200);
  }
  
  Serial.println("   Age: " + String(pet.age) + " (" + String(pet.evolutionStage == 0 ? "Kitten" : (pet.evolutionStage == 1 ? "Young Cat" : "Adult Cat")) + ")");
  Serial.println("   Health: " + String(pet.health) + "%");
}

void handleTouch() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    unsigned long currentTime = millis();
    
    // Prevent multiple rapid touches
    if (currentTime - lastTouch < 200) return;
    lastTouch = currentTime;
    
    TS_Point p = touchscreen.getPoint();
    
    // Convert touch coordinates for rotated display
    int x = map(p.y, 200, 3700, 0, SCREEN_WIDTH);
    int y = map(p.x, 200, 3700, SCREEN_HEIGHT, 0);
    
    Serial.println("👆 Touch detected at: (" + String(x) + ", " + String(y) + ")");
    
    // Check button presses
    for (int i = 0; i < 3; i++) {
      if (x >= buttons[i].x && x <= buttons[i].x + buttons[i].width &&
          y >= buttons[i].y && y <= buttons[i].y + buttons[i].height) {
        
        buttons[i].pressed = true;
        Serial.println("🔘 Button pressed: " + buttons[i].label);
        
        // Handle button actions
        if (buttons[i].label == "FEED") {
          feedCat();
        } else if (buttons[i].label == "PLAY") {
          playCat();
        } else if (buttons[i].label == "RESET") {
          resetCat();
        }
        
        drawScreen();
        delay(100);
        buttons[i].pressed = false;
        drawScreen();
        return;
      }
    }
    
    // Check if touched the cat directly (petting)
    if (x >= PET_X - 40 && x <= PET_X + 40 && 
        y >= PET_Y - 40 && y <= PET_Y + 40) {
      petCat();
      drawScreen();
    }
  }
}

void feedCat() {
  if (pet.hunger > 10) {
    pet.hunger = max(0, pet.hunger - 15);
    pet.lastFeed = millis();
    Serial.println("🍖 Fed " + pet.name + "! Hunger: " + String(pet.hunger) + "%");
    
    // Visual feedback
    tft.setCursor(PET_X - 10, PET_Y - 60);
    tft.setTextColor(TFT_YELLOW, BG_COLOR);
    tft.setTextSize(2);
    tft.print("YUM!");
    delay(1500);
  } else {
    Serial.println("🚫 " + pet.name + " is not hungry!");
    tft.setCursor(PET_X - 20, PET_Y - 60);
    tft.setTextColor(TFT_RED, BG_COLOR);
    tft.setTextSize(1);
    tft.print("Not hungry!");
    delay(1500);
  }
}

void playCat() {
  if (!pet.sleeping) {
    pet.happiness = min(100, pet.happiness + 20);
    pet.hunger = min(100, pet.hunger + 10); // Playing makes hungry!
    pet.lastPlay = millis();
    Serial.println("🎾 Played with " + pet.name + "! Happiness: " + String(pet.happiness) + "%");
    
    // Visual feedback with jumping animation
    tft.setCursor(PET_X - 15, PET_Y - 60);
    tft.setTextColor(TFT_GREEN, BG_COLOR);
    tft.setTextSize(2);
    tft.print("FUN!");
    
    // Simple jump animation
    for (int i = 0; i < 3; i++) {
      drawCat(20, CAT_BODY); // Draw cat higher
      delay(150);
      drawCat(0, CAT_BODY);  // Normal position
      delay(150);
    }
  } else {
    Serial.println("😴 " + pet.name + " is too tired to play!");
    tft.setCursor(PET_X - 25, PET_Y - 60);
    tft.setTextColor(TFT_BLUE, BG_COLOR);
    tft.setTextSize(1);
    tft.print("ZZZ... sleeping");
    delay(1500);
  }
}

void petCat() {
  if (!pet.sleeping) {
    pet.happiness = min(100, pet.happiness + 5);
    Serial.println("👋 Petted " + pet.name + "! Happiness: " + String(pet.happiness) + "%");
    
    // Visual feedback - hearts
    for (int i = 0; i < 3; i++) {
      int heartX = PET_X + random(-30, 30);
      int heartY = PET_Y - 50 + random(-10, 10);
      drawHeart(heartX, heartY);
      delay(300);
    }
    delay(1000);
  }
}

void resetCat() {
  Serial.println("🔄 Resetting " + pet.name + "...");
  
  // Reset flash
  tft.fillScreen(TFT_WHITE);
  delay(300);
  tft.fillScreen(TFT_RED);
  delay(300);
  
  initializePet();
}

void drawScreen() {
  tft.fillScreen(BG_COLOR);
  
  // Title
  tft.setCursor(70, 10);
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.setTextSize(2);
  tft.print("DigiCat");
  
  // Evolution stage
  String stages[] = {"Kitten", "Young Cat", "Adult Cat"};
  tft.setCursor(10, 30);
  tft.setTextSize(1);
  tft.print("Stage: " + stages[pet.evolutionStage] + " (Age: " + String(pet.age) + ")");
  
  // Draw buttons
  for (int i = 0; i < 3; i++) {
    uint16_t color = buttons[i].pressed ? TFT_WHITE : buttons[i].color;
    tft.fillRoundRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, 5, color);
    tft.drawRoundRect(buttons[i].x, buttons[i].y, buttons[i].width, buttons[i].height, 5, UI_COLOR);
    
    // Button text
    tft.setCursor(buttons[i].x + 15, buttons[i].y + 15);
    tft.setTextColor(UI_COLOR, color);
    tft.setTextSize(2);
    tft.print(buttons[i].label);
  }
  
  // Stats bars
  drawStatsBar("HUNGER", pet.hunger, 20, 120, TFT_RED);
  drawStatsBar("HAPPY", pet.happiness, 20, 150, TFT_GREEN);  
  drawStatsBar("HEALTH", pet.health, 20, 180, TFT_BLUE);
  
  // Draw cat
  drawCat();
}

void drawStatsBar(String label, int value, int x, int y, uint16_t color) {
  // Label
  tft.setCursor(x, y);
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.setTextSize(1);
  tft.print(label);
  
  // Bar background
  tft.fillRect(x + 50, y, 100, 15, BAR_BG);
  tft.drawRect(x + 50, y, 100, 15, UI_COLOR);
  
  // Bar fill
  int fillWidth = map(value, 0, 100, 0, 98);
  tft.fillRect(x + 51, y + 1, fillWidth, 13, color);
  
  // Percentage
  tft.setCursor(x + 160, y);
  tft.print(String(value) + "%");
}

void drawCat(int yOffset = 0, uint16_t color = CAT_BODY) {
  int catY = PET_Y + yOffset;
  
  // Clear cat area
  tft.fillRect(PET_X - 50, catY - 50, 100, 80, BG_COLOR);
  
  if (pet.sleeping) {
    drawSleepingCat(20, color, catY);
    return;
  }
  
  drawAwakeCat(20, color, catY);
}

void drawSleepingCat(int size, uint16_t color, int catY = PET_Y) {
  // Main body (large oval)
  tft.fillEllipse(PET_X, catY, size, size*0.7, color);
  
  // Head tucked into body
  tft.fillCircle(PET_X - size*0.3, catY - size*0.2, size*0.4, color);
  
  // Ears poking out
  tft.fillTriangle(PET_X - size*0.5, catY - size*0.4,
                   PET_X - size*0.3, catY - size*0.6,
                   PET_X - size*0.2, catY - size*0.4, color);
  tft.fillTriangle(PET_X - size*0.1, catY - size*0.4,
                   PET_X + 0, catY - size*0.6,
                   PET_X + size*0.1, catY - size*0.4, color);
  
  // Inner ears (pink)
  tft.fillTriangle(PET_X - size*0.45, catY - size*0.45,
                   PET_X - size*0.35, catY - size*0.55,
                   PET_X - size*0.25, catY - size*0.45, CAT_NOSE);
  
  // Closed sleepy eyes (curved lines)
  tft.drawCircle(PET_X - size*0.4, catY - size*0.25, 3, UI_COLOR);
  tft.drawLine(PET_X - size*0.4 - 3, catY - size*0.25, PET_X - size*0.4 + 3, catY - size*0.25, color);
  tft.drawCircle(PET_X - size*0.2, catY - size*0.25, 3, UI_COLOR);
  tft.drawLine(PET_X - size*0.2 - 3, catY - size*0.25, PET_X - size*0.2 + 3, catY - size*0.25, color);
  
  // Tiny nose
  tft.fillCircle(PET_X - size*0.3, catY - size*0.1, 2, CAT_NOSE);
  
  // Tail wrapped around body
  for(int i = 0; i < 8; i++) {
    float angle = i * 0.8;
    int tailX = PET_X + cos(angle) * size * 0.8;
    int tailY = catY + sin(angle) * size * 0.5;
    tft.fillCircle(tailX, tailY, size*0.15, color);
  }
  
  // Stripes on visible body parts
  for(int i = 0; i < 4; i++) {
    tft.drawLine(PET_X - size*0.6 + i*size*0.3, catY - size*0.1, 
                 PET_X - size*0.5 + i*size*0.3, catY + size*0.3, CAT_STRIPE);
  }
  
  // Z z z (bigger)
  tft.setCursor(PET_X + size*0.6, catY - size*0.7);
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.setTextSize(2);
  tft.print("Z");
  tft.setCursor(PET_X + size*0.8, catY - size*0.5);
  tft.setTextSize(1);
  tft.print("z");
  tft.setCursor(PET_X + size*0.9, catY - size*0.3);
  tft.print("z");
}

void drawAwakeCat(int size, uint16_t color, int catY = PET_Y) {
  // Body (different shapes based on evolution)
  if (pet.evolutionStage == 0) {
    // Kitten - smaller, rounder
    tft.fillCircle(PET_X, catY, size*0.8, color);
  } else if (pet.evolutionStage == 1) {
    // Young cat - medium
    tft.fillEllipse(PET_X, catY, size*0.9, size*0.7, color);
  } else {
    // Adult cat - larger, more elegant
    tft.fillEllipse(PET_X, catY, size, size*0.8, color);
  }
  
  // Head
  tft.fillCircle(PET_X, catY - size*0.4, size*0.6, color);
  
  // Ears
  tft.fillTriangle(PET_X - size*0.3, catY - size*0.7,
                   PET_X - size*0.1, catY - size*0.9,
                   PET_X + size*0.1, catY - size*0.7, color);
  tft.fillTriangle(PET_X + size*0.1, catY - size*0.7,
                   PET_X + size*0.3, catY - size*0.9,
                   PET_X + size*0.5, catY - size*0.7, color);
  
  // Inner ears
  tft.fillTriangle(PET_X - size*0.25, catY - size*0.75,
                   PET_X - size*0.15, catY - size*0.85,
                   PET_X - size*0.05, catY - size*0.75, CAT_NOSE);
  
  // Stripes
  for(int i = 0; i < 3; i++) {
    tft.drawLine(PET_X - size*0.6, catY - size*0.2 + i*size*0.3, 
                 PET_X + size*0.6, catY - size*0.1 + i*size*0.3, CAT_STRIPE);
  }
  
  // Tail
  for(int i = 0; i < 6; i++) {
    float angle = i * 0.5;
    int tailX = PET_X + cos(angle) * (size * 0.7 + i * 3);
    int tailY = catY + sin(angle) * size * 0.3;
    tft.fillCircle(tailX, tailY, size*0.1, color);
  }
  
  // Eyes - blinking logic
  if (pet.blinking) {
    // Closed eyes (lines)
    for(int i = 0; i < 3; i++) {
      tft.drawLine(PET_X - size*0.25 - i, catY - size*0.35, 
                   PET_X - size*0.1 + i, catY - size*0.35, UI_COLOR);
      tft.drawLine(PET_X + size*0.1 - i, catY - size*0.35, 
                   PET_X + size*0.2 + i, catY - size*0.35, UI_COLOR);
    }
  } else {
    // Open eyes based on happiness
    if (pet.happiness > 70) {
      // Very happy eyes (big and sparkly)
      tft.fillEllipse(PET_X - size*0.15, catY - size*0.35, 8, 6, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, catY - size*0.35, 8, 6, CAT_EYES);
      // Pupils
      tft.fillEllipse(PET_X - size*0.15, catY - size*0.35, 3, 5, UI_COLOR);
      tft.fillEllipse(PET_X + size*0.15, catY - size*0.35, 3, 5, UI_COLOR);
      // Shine
      tft.fillCircle(PET_X - size*0.15 - 2, catY - size*0.35 - 2, 1, TFT_WHITE);
      tft.fillCircle(PET_X + size*0.15 - 2, catY - size*0.35 - 2, 1, TFT_WHITE);
    } else if (pet.happiness > 30) {
      // Normal eyes
      tft.fillEllipse(PET_X - size*0.15, catY - size*0.35, 6, 5, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, catY - size*0.35, 6, 5, CAT_EYES);
      tft.fillEllipse(PET_X - size*0.15, catY - size*0.35, 2, 4, UI_COLOR);
      tft.fillEllipse(PET_X + size*0.15, catY - size*0.35, 2, 4, UI_COLOR);
    } else {
      // Sad eyes (droopy)
      tft.fillEllipse(PET_X - size*0.15, catY - size*0.32, 5, 4, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, catY - size*0.32, 5, 4, CAT_EYES);
      tft.fillCircle(PET_X - size*0.15, catY - size*0.32, 2, UI_COLOR);
      tft.fillCircle(PET_X + size*0.15, catY - size*0.32, 2, UI_COLOR);
    }
  }
  
  // Nose
  tft.fillTriangle(PET_X - 2, catY - size*0.2,
                   PET_X, catY - size*0.15,
                   PET_X + 2, catY - size*0.2, CAT_NOSE);
  
  // Mouth
  if (pet.happiness > 70) {
    // Happy mouth
    tft.drawCircle(PET_X, catY - size*0.1, 8, UI_COLOR);
    tft.fillRect(PET_X - 8, catY - size*0.1 - 8, 16, 8, color);
  } else if (pet.happiness < 30) {
    // Sad mouth
    tft.drawCircle(PET_X, catY - size*0.05, 6, UI_COLOR);
    tft.fillRect(PET_X - 6, catY - size*0.05, 12, 6, color);
  }
  
  // Whiskers
  for(int i = 0; i < 3; i++) {
    // Left whiskers
    tft.drawLine(PET_X - size*0.4, catY - size*0.2 + i*3, 
                 PET_X - size*0.1, catY - size*0.2 + i*3, UI_COLOR);
    // Right whiskers  
    tft.drawLine(PET_X + size*0.1, catY - size*0.2 + i*3,
                 PET_X + size*0.4, catY - size*0.2 + i*3, UI_COLOR);
  }
}

void drawHeart(int x, int y) {
  // Simple heart shape
  tft.fillCircle(x - 3, y - 2, 3, TFT_RED);
  tft.fillCircle(x + 3, y - 2, 3, TFT_RED);
  tft.fillTriangle(x - 6, y, x, y + 6, x + 6, y, TFT_RED);
}
