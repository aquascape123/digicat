/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║                           DigiCat v1.0.0                        ║
 * ║                    Virtual Pet for ESP32 CYD-R                  ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║                                                                  ║
 * ║  🐱 A charming virtual cat companion with pixel art animations  ║
 * ║  📱 Touch interface for feeding, playing, and petting          ║
 * ║  🏆 Evolution system from kitten to adult cat                  ║
 * ║  😴 Dynamic moods, expressions, and sleep cycles               ║
 * ║  🎮 Chill gameplay with realistic stat progression             ║
 * ║                                                                  ║
 * ╠══════════════════════════════════════════════════════════════════╣
 * ║  Hardware: ESP32 CYD-R (2432S024R) - Resistive Touch Only      ║
 * ║  Display:  240x320 ST7789 TFT                                  ║
 * ║  Touch:    XPT2046 Resistive Controller                        ║
 * ║                                                                  ║
 * ║  Libraries Required:                                            ║
 * ║  - TFT_eSPI by Bodmer                                          ║
 * ║  - XPT2046_Touchscreen by Paul Stoffregen                     ║
 * ║                                                                  ║
 * ║  GitHub: https://github.com/aquascape123/digicat               ║
 * ║  License: MIT                                                   ║
 * ║                                                                  ║
 * ║  Made with ❤️ for the ESP32 maker community                    ║
 * ╚══════════════════════════════════════════════════════════════════╝
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

TFT_eSPI tft = TFT_eSPI();

// Touchscreen setup for ESP32 CYD-R (2432S024R)
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

// UI Layout for 240x320 portrait display
const int SCREEN_WIDTH = 240;   
const int SCREEN_HEIGHT = 320;
const int PET_X = 80;           // Cat position (lower-left area)
const int PET_Y = 260;          

// Touch button definitions (3 buttons total)
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

// Color definitions
const uint16_t BG_COLOR = TFT_CYAN;
const uint16_t UI_COLOR = TFT_BLACK;
const uint16_t BAR_BG = TFT_WHITE;

// Cat colors
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
  Serial.println("║        Virtual Pet for ESP32 CYD-R       ║");
  Serial.println("╚══════════════════════════════════════════╝");
  Serial.println();
  
  // Initialize touchscreen with dedicated SPI
  Serial.println("🔧 Initializing touchscreen...");
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  Serial.println("✅ Touchscreen ready!");
  
  // Initialize display (portrait mode rotated 180°)
  Serial.println("🖥️  Initializing display...");
  tft.init();
  tft.setRotation(2); // Portrait rotated 180 degrees
  Serial.println("✅ Display ready!");
  
  // Initialize pet
  Serial.println("🐱 Initializing DigiCat...");
  initializePet();
  Serial.println("✅ DigiCat ready!");
  
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
  
  // Blink animation every 3-6 seconds (more relaxed)
  if (currentTime - pet.lastBlink > random(3000, 6000)) {
    pet.blinking = true;
    pet.lastBlink = currentTime;
    // Only redraw the cat, not the whole screen
    drawCat();
  }
  
  // Stop blinking after 200ms
  if (pet.blinking && (currentTime - pet.lastBlink > 200)) {
    pet.blinking = false;
    // Only redraw the cat, not the whole screen
    drawCat();
  }
  
  // Animation frame for idle movements (slower) - removed automatic redraw
  if (currentTime % 8000 < 100) { // Even slower - every 8 seconds
    pet.animationFrame = (pet.animationFrame + 1) % 3;
    // Only redraw cat when animation changes
    drawCat();
  }
  
  // Handle touch
  handleTouch();
  
  delay(100); // Slightly slower polling for more chill experience
}

void initializePet() {
  pet.name = "DigiCat";
  pet.happiness = 50;
  pet.hunger = 30;
  pet.health = 80;
  pet.age = 0;
  pet.evolutionStage = 0; // Kitten
  pet.lastUpdate = millis();
  pet.lastFeed = millis();
  pet.lastPlay = millis();
  pet.sleeping = false;
  pet.blinking = false;
  pet.lastBlink = millis();
  pet.animationFrame = 0;
}

void updatePetStats() {
  // Hunger increases very slowly over time (much more chill!)
  if (millis() - pet.lastFeed > 300000) { // 5 minutes instead of 15 seconds
    pet.hunger = min(100, pet.hunger + 1); // Only +1 instead of +3
    pet.happiness = max(0, pet.happiness - 1);
  }
  
  // Happiness decreases very slowly without play
  if (millis() - pet.lastPlay > 400000) { // 6.6 minutes instead of 20 seconds
    pet.happiness = max(0, pet.happiness - 1); // Only -1 instead of -2
  }
  
  // Health depends on hunger and happiness (gentler changes)
  if (pet.hunger > 90 || pet.happiness < 10) {
    pet.health = max(0, pet.health - 1); // Only -1 instead of -3
  } else if (pet.hunger < 30 && pet.happiness > 70) {
    pet.health = min(100, pet.health + 1); // Same slow recovery
  }
  
  // Cat sleeps only if health is very low
  pet.sleeping = (pet.health < 15); // Lower threshold for sleeping
  
  // Evolution based on age (slower aging)
  pet.age = millis() / 120000; // Age in 2-minute units instead of 30-second units
  
  if (pet.age < 5) pet.evolutionStage = 0;      // Kitten (0-10 minutes)
  else if (pet.age < 15) pet.evolutionStage = 1; // Young cat (10-30 minutes)
  else pet.evolutionStage = 2;                   // Adult cat (30+ minutes)
  
  // Debug stats
  Serial.println("Hunger:" + String(pet.hunger) + 
                " Happiness:" + String(pet.happiness) + 
                " Health:" + String(pet.health) +
                " Age:" + String(pet.age) +
                " Stage:" + String(pet.evolutionStage));
}

void handleTouch() {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    // Debounce touch
    if (millis() - lastTouch < 300) return;
    
    TS_Point p = touchscreen.getPoint();
    
    // Calibration adjusted for 180° rotation (both X and Y inverted)
    int touchX = map(p.x, 240, 3700, 0, SCREEN_WIDTH);   // Normal X (was inverted)
    int touchY = map(p.y, 200, 3800, SCREEN_HEIGHT, 0);  // Inverted Y (was normal)
    
    // Constrain coordinates
    touchX = constrain(touchX, 0, SCREEN_WIDTH-1);
    touchY = constrain(touchY, 0, SCREEN_HEIGHT-1);
    
    Serial.printf("Touch: x=%d, y=%d (raw: %d,%d)\n", touchX, touchY, p.x, p.y);
    
    // Check which button was pressed
    for (int i = 0; i < 3; i++) {
      if (isButtonPressed(touchX, touchY, buttons[i])) {
        Serial.printf("Button %d (%s) pressed!\n", i, buttons[i].label.c_str());
        handleButtonPress(i);
        lastTouch = millis();
        return;
      }
    }
    
    // If touching the cat, pet it
    if (isCatTouched(touchX, touchY)) {
      petCat();
      lastTouch = millis();
    }
  }
}

bool isButtonPressed(int x, int y, TouchButton btn) {
  return (x >= btn.x && x <= btn.x + btn.width && 
          y >= btn.y && y <= btn.y + btn.height);
}

bool isCatTouched(int x, int y) {
  int catSize = getCatSize();
  return (abs(x - PET_X) <= catSize && abs(y - PET_Y) <= catSize);
}

int getCatSize() {
  // Much bigger cat sizes!
  if (pet.evolutionStage == 0) return 35;      // Bigger kitten
  else if (pet.evolutionStage == 1) return 45; // Much bigger young cat
  else return 55;                              // Large adult cat
}

void handleButtonPress(int buttonIndex) {
  Serial.println("Button pressed: " + buttons[buttonIndex].label);
  
  // Visual feedback
  buttons[buttonIndex].pressed = true;
  drawButtons();
  delay(100);
  buttons[buttonIndex].pressed = false;
  
  switch (buttonIndex) {
    case 0: // FEED
      feedCat();
      break;
    case 1: // PLAY
      playWithCat();
      break;
    case 2: // RESET
      resetCat();
      break;
  }
  
  drawScreen();
}

void petCat() {
  Serial.println("Petting the cat!");
  
  if (!pet.sleeping) {
    pet.happiness = min(100, pet.happiness + 8); // More generous petting reward
    
    // Heart animation (adjusted for new cat position)
    for (int i = 0; i < 3; i++) {
      // Draw heart above cat - positioned for left-bottom placement
      int heartY = PET_Y - 40;
      tft.fillCircle(PET_X - 5, heartY, 3, TFT_RED);
      tft.fillCircle(PET_X + 5, heartY, 3, TFT_RED);
      tft.fillTriangle(PET_X - 8, heartY + 2, PET_X + 8, heartY + 2, PET_X, heartY + 10, TFT_RED);
      delay(400); // Slower, more relaxed heart animation
      
      // Clear heart
      tft.fillCircle(PET_X - 5, heartY, 4, BG_COLOR);
      tft.fillCircle(PET_X + 5, heartY, 4, BG_COLOR);
      tft.fillTriangle(PET_X - 9, heartY + 1, PET_X + 9, heartY + 1, PET_X, heartY + 11, BG_COLOR);
      delay(300);
    }
  }
  
  drawScreen();
}

void drawScreen() {
  // Background
  tft.fillScreen(BG_COLOR);
  
  // Title
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.setTextSize(2);
  tft.setCursor(10, 5);
  tft.println(pet.name);
  
  // Subtitle with stage
  tft.setTextSize(1);
  tft.setCursor(10, 25);
  String stageText = "Stage: ";
  if (pet.evolutionStage == 0) stageText += "Kitten";
  else if (pet.evolutionStage == 1) stageText += "Young Cat";
  else stageText += "Adult Cat";
  stageText += " (Age: " + String(pet.age) + ")";
  tft.println(stageText);
  
  // Buttons
  drawButtons();
  
  // Stats bars
  drawStats();
  
  // Cat in center
  drawCat();
  
  // Instructions (positioned to not overlap with anything)
  tft.setTextSize(1);
  tft.setCursor(5, 300);
  tft.setTextColor(TFT_BLACK, BG_COLOR);
  tft.println("Touch buttons or pet!");
}

void drawButtons() {
  for (int i = 0; i < 3; i++) {
    TouchButton btn = buttons[i];
    
    // Button color (darker if pressed)
    uint16_t color = btn.pressed ? TFT_DARKGREY : btn.color;
    
    // Draw button
    tft.fillRoundRect(btn.x, btn.y, btn.width, btn.height, 8, color);
    tft.drawRoundRect(btn.x, btn.y, btn.width, btn.height, 8, UI_COLOR);
    
    // Button text
    tft.setTextColor(UI_COLOR, color);
    tft.setTextSize(2);  // Bigger text for bigger buttons
    
    // Center text
    int textX = btn.x + (btn.width - btn.label.length() * 12) / 2;
    int textY = btn.y + (btn.height - 16) / 2;
    tft.setCursor(textX, textY);
    tft.print(btn.label);
  }
}

void drawStats() {
  int startY = 110;
  int barWidth = 180;  
  int barHeight = 10;   
  
  // Hunger
  tft.setTextSize(1);
  tft.setCursor(10, startY);
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.print("Hunger: " + String(pet.hunger) + "%");
  drawBar(10, startY + 12, pet.hunger, TFT_RED, barWidth, barHeight);
  
  // Happiness
  tft.setCursor(10, startY + 30);
  tft.print("Happiness: " + String(pet.happiness) + "%");
  drawBar(10, startY + 42, pet.happiness, TFT_YELLOW, barWidth, barHeight);
  
  // Health
  tft.setCursor(10, startY + 60);
  tft.print("Health: " + String(pet.health) + "%");
  drawBar(10, startY + 72, pet.health, TFT_GREEN, barWidth, barHeight);
}

void drawBar(int x, int y, int value, uint16_t color, int width, int height) {
  // Frame
  tft.drawRect(x, y, width + 2, height + 2, UI_COLOR);
  tft.fillRect(x + 1, y + 1, width, height, BAR_BG);
  
  // Fill
  int fillWidth = map(value, 0, 100, 0, width);
  tft.fillRect(x + 1, y + 1, fillWidth, height, color);
}

void drawCat() {
  // Clear only the cat area with precise boundaries for new position
  int catSize = getCatSize();
  
  // Calculate safe clear area for new left-bottom position
  int clearWidth = catSize + 25;  // A bit more for whiskers and tail
  int clearHeight = catSize + 20;
  int clearX = PET_X - clearWidth/2;
  int clearY = PET_Y - clearHeight/2;
  
  // Make sure we don't clear above Y=200 (well below health bar) 
  if (clearY < 200) {
    clearHeight = clearHeight - (200 - clearY);
    clearY = 200;
  }
  
  // Make sure we don't go off left edge
  if (clearX < 0) {
    clearWidth = clearWidth + clearX;
    clearX = 0;
  }
  
  // Make sure we don't clear into reset button area (X>150, Y>265)
  if (clearX + clearWidth > 150 && clearY + clearHeight > 265) {
    if (clearX + clearWidth > 150) {
      clearWidth = 150 - clearX;
    }
  }
  
  // Clear the safe area only
  tft.fillRect(clearX, clearY, clearWidth, clearHeight, BG_COLOR);
  
  int bodyColor = CAT_BODY;
  
  // Color based on health
  if (pet.health < 30) bodyColor = TFT_DARKGREY;
  else if (pet.health < 60) bodyColor = TFT_ORANGE;
  
  if (pet.sleeping) {
    drawSleepingCat(catSize, bodyColor);
  } else {
    drawAwakeCat(catSize, bodyColor);
  }
}

void drawSleepingCat(int size, uint16_t color) {
  // Sleeping cat curled up in a ball - much cuter!
  
  // Main body (large oval)
  tft.fillEllipse(PET_X, PET_Y, size, size*0.7, color);
  
  // Head tucked into body
  tft.fillCircle(PET_X - size*0.3, PET_Y - size*0.2, size*0.4, color);
  
  // Ears poking out
  tft.fillTriangle(PET_X - size*0.5, PET_Y - size*0.4,
                   PET_X - size*0.3, PET_Y - size*0.6,
                   PET_X - size*0.2, PET_Y - size*0.4, color);
  tft.fillTriangle(PET_X - size*0.1, PET_Y - size*0.4,
                   PET_X + 0, PET_Y - size*0.6,
                   PET_X + size*0.1, PET_Y - size*0.4, color);
  
  // Inner ears (pink)
  tft.fillTriangle(PET_X - size*0.45, PET_Y - size*0.45,
                   PET_X - size*0.35, PET_Y - size*0.55,
                   PET_X - size*0.25, PET_Y - size*0.45, CAT_NOSE);
  
  // Closed sleepy eyes (curved lines)
  tft.drawCircle(PET_X - size*0.4, PET_Y - size*0.25, 3, UI_COLOR);
  tft.drawLine(PET_X - size*0.4 - 3, PET_Y - size*0.25, PET_X - size*0.4 + 3, PET_Y - size*0.25, color);
  tft.drawCircle(PET_X - size*0.2, PET_Y - size*0.25, 3, UI_COLOR);
  tft.drawLine(PET_X - size*0.2 - 3, PET_Y - size*0.25, PET_X - size*0.2 + 3, PET_Y - size*0.25, color);
  
  // Tiny nose
  tft.fillCircle(PET_X - size*0.3, PET_Y - size*0.1, 2, CAT_NOSE);
  
  // Tail wrapped around body
  for(int i = 0; i < 8; i++) {
    float angle = i * 0.8;
    int tailX = PET_X + cos(angle) * size * 0.8;
    int tailY = PET_Y + sin(angle) * size * 0.5;
    tft.fillCircle(tailX, tailY, size*0.15, color);
  }
  
  // Stripes on visible body parts
  for(int i = 0; i < 4; i++) {
    tft.drawLine(PET_X - size*0.6 + i*size*0.3, PET_Y - size*0.1, 
                 PET_X - size*0.5 + i*size*0.3, PET_Y + size*0.3, CAT_STRIPE);
  }
  
  // Z z z (bigger)
  tft.setCursor(PET_X + size*0.6, PET_Y - size*0.7);
  tft.setTextColor(UI_COLOR, BG_COLOR);
  tft.setTextSize(2);
  tft.print("Z");
  tft.setCursor(PET_X + size*0.8, PET_Y - size*0.5);
  tft.setTextSize(1);
  tft.print("z");
  tft.setCursor(PET_X + size*0.9, PET_Y - size*0.3);
  tft.print("z");
}

void drawAwakeCat(int size, uint16_t color) {
  // Much more detailed and cute awake cat!
  
  // Main body (oval with better proportions)
  tft.fillEllipse(PET_X, PET_Y + size*0.1, size*0.8, size*0.5, color);
  
  // Chest/belly (lighter oval)
  uint16_t bellyColor = TFT_WHITE;
  if (color == TFT_DARKGREY) bellyColor = TFT_LIGHTGREY;
  tft.fillEllipse(PET_X, PET_Y + size*0.2, size*0.5, size*0.3, bellyColor);
  
  // Head (proper cat head shape)
  tft.fillCircle(PET_X, PET_Y - size*0.3, size*0.4, color);
  
  // Cheeks (make face rounder)
  tft.fillCircle(PET_X - size*0.25, PET_Y - size*0.25, size*0.2, color);
  tft.fillCircle(PET_X + size*0.25, PET_Y - size*0.25, size*0.2, color);
  
  // Ears (better proportioned)
  tft.fillTriangle(PET_X - size*0.3, PET_Y - size*0.65, 
                   PET_X - size*0.1, PET_Y - size*0.45,
                   PET_X - size*0.05, PET_Y - size*0.65, color);
  tft.fillTriangle(PET_X + size*0.05, PET_Y - size*0.65, 
                   PET_X + size*0.1, PET_Y - size*0.45,
                   PET_X + size*0.3, PET_Y - size*0.65, color);
  
  // Inner ears (pink)
  tft.fillTriangle(PET_X - size*0.25, PET_Y - size*0.6, 
                   PET_X - size*0.15, PET_Y - size*0.5,
                   PET_X - size*0.1, PET_Y - size*0.6, CAT_NOSE);
  tft.fillTriangle(PET_X + size*0.1, PET_Y - size*0.6, 
                   PET_X + size*0.15, PET_Y - size*0.5,
                   PET_X + size*0.25, PET_Y - size*0.6, CAT_NOSE);
  
  // Eyes (much better and bigger)
  if (pet.blinking) {
    // Closed eyes (cute curved lines)
    for(int i = 0; i < 3; i++) {
      tft.drawLine(PET_X - size*0.2 - i, PET_Y - size*0.35, 
                   PET_X - size*0.1 + i, PET_Y - size*0.35, UI_COLOR);
      tft.drawLine(PET_X + size*0.1 - i, PET_Y - size*0.35, 
                   PET_X + size*0.2 + i, PET_Y - size*0.35, UI_COLOR);
    }
  } else {
    // Open eyes based on happiness
    if (pet.happiness > 70) {
      // Very happy eyes (big and sparkly)
      tft.fillEllipse(PET_X - size*0.15, PET_Y - size*0.35, 8, 6, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, PET_Y - size*0.35, 8, 6, CAT_EYES);
      // Pupils
      tft.fillEllipse(PET_X - size*0.15, PET_Y - size*0.35, 3, 5, UI_COLOR);
      tft.fillEllipse(PET_X + size*0.15, PET_Y - size*0.35, 3, 5, UI_COLOR);
      // Shine
      tft.fillCircle(PET_X - size*0.15 - 2, PET_Y - size*0.35 - 2, 1, TFT_WHITE);
      tft.fillCircle(PET_X + size*0.15 - 2, PET_Y - size*0.35 - 2, 1, TFT_WHITE);
    } else if (pet.happiness > 30) {
      // Normal eyes
      tft.fillEllipse(PET_X - size*0.15, PET_Y - size*0.35, 6, 5, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, PET_Y - size*0.35, 6, 5, CAT_EYES);
      tft.fillEllipse(PET_X - size*0.15, PET_Y - size*0.35, 2, 4, UI_COLOR);
      tft.fillEllipse(PET_X + size*0.15, PET_Y - size*0.35, 2, 4, UI_COLOR);
    } else {
      // Sad eyes (droopy)
      tft.fillEllipse(PET_X - size*0.15, PET_Y - size*0.32, 5, 4, CAT_EYES);
      tft.fillEllipse(PET_X + size*0.15, PET_Y - size*0.32, 5, 4, CAT_EYES);
      tft.fillCircle(PET_X - size*0.15, PET_Y - size*0.32, 2, UI_COLOR);
      tft.fillCircle(PET_X + size*0.15, PET_Y - size*0.32, 2, UI_COLOR);
    }
  }
  
  // Nose (triangle with gradient effect)
  tft.fillTriangle(PET_X - 2, PET_Y - size*0.18, 
                   PET_X + 2, PET_Y - size*0.18, 
                   PET_X, PET_Y - size*0.12, CAT_NOSE);
  tft.drawTriangle(PET_X - 2, PET_Y - size*0.18, 
                   PET_X + 2, PET_Y - size*0.18, 
                   PET_X, PET_Y - size*0.12, TFT_RED);
  
  // Mouth (much better)
  if (pet.happiness > 60) {
    // Happy mouth (smile)
    tft.drawLine(PET_X, PET_Y - size*0.12, PET_X - 3, PET_Y - size*0.08, UI_COLOR);
    tft.drawLine(PET_X, PET_Y - size*0.12, PET_X + 3, PET_Y - size*0.08, UI_COLOR);
  } else if (pet.happiness > 30) {
    // Neutral mouth
    tft.drawLine(PET_X - 2, PET_Y - size*0.08, PET_X + 2, PET_Y - size*0.08, UI_COLOR);
  } else {
    // Sad mouth (frown)
    tft.drawLine(PET_X, PET_Y - size*0.05, PET_X - 3, PET_Y - size*0.08, UI_COLOR);
    tft.drawLine(PET_X, PET_Y - size*0.05, PET_X + 3, PET_Y - size*0.08, UI_COLOR);
  }
  
  // Whiskers (longer and more realistic)
  tft.drawLine(PET_X - size*0.4, PET_Y - size*0.2, PET_X - size*0.15, PET_Y - size*0.15, UI_COLOR);
  tft.drawLine(PET_X - size*0.4, PET_Y - size*0.1, PET_X - size*0.15, PET_Y - size*0.1, UI_COLOR);
  tft.drawLine(PET_X - size*0.4, PET_Y, PET_X - size*0.15, PET_Y - size*0.05, UI_COLOR);
  
  tft.drawLine(PET_X + size*0.15, PET_Y - size*0.15, PET_X + size*0.4, PET_Y - size*0.2, UI_COLOR);
  tft.drawLine(PET_X + size*0.15, PET_Y - size*0.1, PET_X + size*0.4, PET_Y - size*0.1, UI_COLOR);
  tft.drawLine(PET_X + size*0.15, PET_Y - size*0.05, PET_X + size*0.4, PET_Y, UI_COLOR);
  
  // Front paws (visible when sitting)
  tft.fillEllipse(PET_X - size*0.2, PET_Y + size*0.35, size*0.12, size*0.08, color);
  tft.fillEllipse(PET_X + size*0.2, PET_Y + size*0.35, size*0.12, size*0.08, color);
  
  // Paw pads
  tft.fillCircle(PET_X - size*0.2, PET_Y + size*0.35, 2, CAT_NOSE);
  tft.fillCircle(PET_X + size*0.2, PET_Y + size*0.35, 2, CAT_NOSE);
  
  // Tail (animated and more detailed)
  int tailOffset = pet.animationFrame * 3;
  float tailCurve = sin(millis() * 0.001) * 0.1;
  
  for(int i = 0; i < 6; i++) {
    int tailX = PET_X + size*0.4 + i*5 + tailOffset;
    int tailY = PET_Y + size*0.1 - i*3 + sin(i*0.5 + tailCurve)*5;
    tft.fillCircle(tailX, tailY, size*0.08 - i, color);
  }
  
  // Stripes on body (more realistic pattern)
  for(int i = 0; i < 5; i++) {
    // Body stripes
    tft.drawLine(PET_X - size*0.3 + i*size*0.15, PET_Y - size*0.1, 
                 PET_X - size*0.2 + i*size*0.15, PET_Y + size*0.3, CAT_STRIPE);
    // Head stripes  
    if (i < 3) {
      tft.drawLine(PET_X - size*0.2 + i*size*0.2, PET_Y - size*0.55, 
                   PET_X - size*0.1 + i*size*0.2, PET_Y - size*0.4, CAT_STRIPE);
    }
  }
  
  // Forehead marking (classic "M" shape)
  tft.drawLine(PET_X - size*0.1, PET_Y - size*0.5, PET_X, PET_Y - size*0.45, CAT_STRIPE);
  tft.drawLine(PET_X, PET_Y - size*0.45, PET_X + size*0.1, PET_Y - size*0.5, CAT_STRIPE);
  tft.drawLine(PET_X, PET_Y - size*0.55, PET_X, PET_Y - size*0.45, CAT_STRIPE);
}

void feedCat() {
  Serial.println("Feeding the cat!");
  pet.hunger = max(0, pet.hunger - 30); // More effective feeding
  pet.happiness = min(100, pet.happiness + 15); // More happiness from food
  pet.lastFeed = millis();
  
  // Food bowl animation (adjusted for new cat position)
  for (int i = 0; i < 3; i++) {
    // Draw food bowl next to cat (not below to avoid going off screen)
    int bowlX = PET_X + getCatSize() + 15; // To the right of cat
    int bowlY = PET_Y + 10; // Slightly below cat center
    
    // Make sure bowl stays on screen
    if (bowlX < SCREEN_WIDTH - 20 && bowlY < SCREEN_HEIGHT - 10) {
      tft.fillEllipse(bowlX, bowlY, 8, 4, TFT_BROWN);
      tft.fillCircle(bowlX - 3, bowlY - 3, 2, TFT_RED); // Food
      tft.fillCircle(bowlX + 3, bowlY - 3, 2, TFT_RED);
      delay(400);
      
      // Clear food bowl
      tft.fillEllipse(bowlX, bowlY, 10, 6, BG_COLOR);
    }
    delay(300);
  }
}

void playWithCat() {
  Serial.println("Playing with the cat!");
  pet.happiness = min(100, pet.happiness + 25); // More happiness from play
  pet.hunger = min(100, pet.hunger + 3); // Less hunger from playing (more chill)
  pet.lastPlay = millis();
  
  // Jumping animation - now safe with lower-left position
  int catSize = getCatSize();
  for (int i = 0; i < 3; i++) { // Only 3 jumps instead of 4
    // Clear original position
    tft.fillRect(PET_X - catSize, PET_Y - catSize, catSize*2, catSize*2, BG_COLOR);
    
    // Draw cat jumping up (safe jump height for new position)
    int jumpY = PET_Y - 20; // Jump height safe now that cat is at Y=260
    drawJumpingCat(jumpY, catSize, CAT_BODY);
    delay(300); // Slower, more relaxed animation
    
    // Clear jump position
    tft.fillRect(PET_X - catSize, jumpY - catSize, catSize*2, catSize*2, BG_COLOR);
    
    // Cat back down - redraw at original position
    drawAwakeCat(catSize, CAT_BODY);
    delay(300); // Slower timing
  }
}

void drawJumpingCat(int yPos, int size, uint16_t color) {
  // Simplified jumping cat (excited version of awake cat)
  
  // Main body (slightly stretched for jumping)
  tft.fillEllipse(PET_X, yPos + size*0.1, size*0.8, size*0.4, color);
  
  // Head
  tft.fillCircle(PET_X, yPos - size*0.3, size*0.4, color);
  
  // Ears (perked up with excitement)
  tft.fillTriangle(PET_X - size*0.3, yPos - size*0.7, 
                   PET_X - size*0.1, yPos - size*0.45,
                   PET_X - size*0.05, yPos - size*0.7, color);
  tft.fillTriangle(PET_X + size*0.05, yPos - size*0.7, 
                   PET_X + size*0.1, yPos - size*0.45,
                   PET_X + size*0.3, yPos - size*0.7, color);
  
  // Excited eyes (always big and happy when jumping)
  tft.fillEllipse(PET_X - size*0.15, yPos - size*0.35, 8, 6, CAT_EYES);
  tft.fillEllipse(PET_X + size*0.15, yPos - size*0.35, 8, 6, CAT_EYES);
  tft.fillEllipse(PET_X - size*0.15, yPos - size*0.35, 3, 5, UI_COLOR);
  tft.fillEllipse(PET_X + size*0.15, yPos - size*0.35, 3, 5, UI_COLOR);
  tft.fillCircle(PET_X - size*0.15 - 2, yPos - size*0.35 - 2, 1, TFT_WHITE);
  tft.fillCircle(PET_X + size*0.15 - 2, yPos - size*0.35 - 2, 1, TFT_WHITE);
  
  // Happy nose
  tft.fillTriangle(PET_X - 2, yPos - size*0.18, 
                   PET_X + 2, yPos - size*0.18, 
                   PET_X, yPos - size*0.12, CAT_NOSE);
  
  // Excited mouth (big smile)
  tft.drawLine(PET_X, yPos - size*0.12, PET_X - 4, yPos - size*0.06, UI_COLOR);
  tft.drawLine(PET_X, yPos - size*0.12, PET_X + 4, yPos - size*0.06, UI_COLOR);
  
  // Extended paws (jumping pose)
  tft.fillEllipse(PET_X - size*0.3, yPos + size*0.4, size*0.12, size*0.08, color);
  tft.fillEllipse(PET_X + size*0.3, yPos + size*0.4, size*0.12, size*0.08, color);
  
  // Tail up with excitement
  for(int i = 0; i < 4; i++) {
    int tailX = PET_X + size*0.4 + i*3;
    int tailY = yPos - size*0.2 - i*5;
    tft.fillCircle(tailX, tailY, size*0.08, color);
  }
}

void resetCat() {
  Serial.println("Resetting cat!");
  initializePet();
  
  // Reset flash
  tft.fillScreen(TFT_WHITE);
  delay(300);
  tft.fillScreen(TFT_RED);
  delay(300);
}
