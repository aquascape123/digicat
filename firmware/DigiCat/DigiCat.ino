/*
 * DigiCat - a tiny virtual cat for the ESP32 "Cheap Yellow Display"
 * https://github.com/aquascape123/digicat  (MIT License)
 *
 * Boards:   ESP32-2432S024R (2.4") or ESP32-2432S028R (2.8"), pick one in config.h
 * Library:  LovyanGFX (Library Manager). No setup files to copy.
 * Arduino:  board "ESP32 Dev Module"
 *
 * First boot asks you to tap the four screen corners (touch calibration).
 * Hold your finger on the screen while the splash is shown to calibrate again.
 */
#include <Preferences.h>
#include "config.h"
#include "display.h"
#include "art.h"
#include "pet.h"
#include "ui.h"

LGFX lcd;
Pet pet;
Ui ui(lcd, pet);

static void calibrateTouch(bool force) {
  Preferences prefs;
  prefs.begin("touch", false);
  uint16_t cal[8];
  if (!force && prefs.getBytes("cal", cal, sizeof(cal)) == sizeof(cal)) {
    lcd.setTouchCalibrate(cal);
  } else {
    lcd.fillScreen(pal::paper);
    lcd.setTextColor(pal::ink);
    lcd.setTextDatum(textdatum_t::middle_center);
    lcd.setFont(&fonts::FreeSansBold9pt7b);
    lcd.drawString("Touch calibration", lcd.width() / 2, lcd.height() / 2 - 12);
    lcd.setFont(&fonts::Font2);
    lcd.drawString("Tap each arrow tip", lcd.width() / 2, lcd.height() / 2 + 12);
    lcd.calibrateTouch(cal, pal::red, pal::paper, 15);
    prefs.putBytes("cal", cal, sizeof(cal));
    Serial.println("[touch] calibration saved");
  }
  prefs.end();
}

static void splash() {
  lcd.fillScreen(pal::paper);
  CatPose pose;
  pose.eyes = CatPose::Eyes::Happy;
  pose.blush = true;
  drawCatSitting(lcd, lcd.width() / 2, 190, pose);
  lcd.setTextColor(pal::ink);
  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setFont(&fonts::FreeSansBold18pt7b);
  lcd.drawString("DigiCat", lcd.width() / 2, 230);
  lcd.setFont(&fonts::Font2);
  lcd.setTextColor(rgb(150, 140, 160));
  lcd.drawString("v" DIGICAT_VERSION, lcd.width() / 2, 262);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nDigiCat v" DIGICAT_VERSION);
  randomSeed(esp_random());

  lcd.init();
  lcd.setRotation(SCREEN_ROTATION);
  lcd.setBrightness(BRIGHTNESS_ACTIVE);

  splash();
  bool held = false;
  for (uint32_t t0 = millis(); millis() - t0 < 1500;) {
    int32_t x, y;
    if (lcd.getTouch(&x, &y)) held = true;
    delay(20);
  }
  calibrateTouch(held);

  pet.begin();
  ui.begin();
}

void loop() {
  pet.update();
  ui.update();
  pet.saveIfDue();
  delay(2);  // let the idle task breathe
}
