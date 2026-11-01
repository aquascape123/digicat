// DigiCat - vector art: the cat and small icons. Draws on the screen or a sprite.
#pragma once
#include "display.h"

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
  return uint16_t(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

namespace pal {
constexpr uint16_t ink      = rgb(40, 34, 48);
constexpr uint16_t white    = rgb(255, 255, 255);
constexpr uint16_t paper    = rgb(255, 248, 238);
constexpr uint16_t pink     = rgb(255, 150, 170);
constexpr uint16_t blush    = rgb(255, 180, 190);
constexpr uint16_t red      = rgb(235, 70, 80);
constexpr uint16_t green    = rgb(90, 200, 120);
constexpr uint16_t yellow   = rgb(250, 200, 60);
constexpr uint16_t blue     = rgb(80, 150, 240);
constexpr uint16_t eye      = rgb(120, 200, 90);
constexpr uint16_t furOk    = rgb(247, 160, 70);
constexpr uint16_t stripeOk = rgb(205, 110, 40);
constexpr uint16_t furSick  = rgb(185, 170, 155);
constexpr uint16_t stripeSick = rgb(140, 125, 115);
constexpr uint16_t belly    = rgb(255, 238, 215);
}  // namespace pal

struct CatPose {
  enum class Eyes : uint8_t { Open, Closed, Happy, Sad };
  enum class Mouth : uint8_t { Smile, Neutral, Frown, Open };

  float    scale   = 1.0f;
  uint16_t fur     = pal::furOk;
  uint16_t stripe  = pal::stripeOk;
  Eyes     eyes    = Eyes::Open;
  Mouth    mouth   = Mouth::Smile;
  int8_t   look    = 0;      // pupil offset, -3..3 px
  float    tail    = 0;      // tail sway, -1..1
  uint8_t  breathe = 0;      // 0..2 px
  bool     blush   = false;
};

// (cx, floorY) is the point between the front paws, on the floor
void drawCatSitting(lgfx::LovyanGFX& g, int cx, int floorY, const CatPose& p);
void drawCatSleeping(lgfx::LovyanGFX& g, int cx, int floorY, const CatPose& p);

void drawHeart(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c);
void drawFish(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c);
void drawBall(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c);
void drawBolt(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c);
void drawMoon(lgfx::LovyanGFX& g, int x, int y, int r, uint16_t c, uint16_t bg);
void drawBowl(lgfx::LovyanGFX& g, int x, int floorY, int fill);  // fill 0..3
