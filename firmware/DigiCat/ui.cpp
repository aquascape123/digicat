#include "ui.h"
#include "art.h"
#include "config.h"

namespace {

// 240 x 320 portrait layout
constexpr int W = 240;
constexpr int HEADER_H = 34;
constexpr int STATS_Y = 38;
constexpr int STAGE_Y = 104;
constexpr int STAGE_H = 158;
constexpr int BTN_Y = 268;
constexpr int BTN_H = 46;
constexpr int BTN_W = 72;
constexpr int BTN_X[3] = {6, 84, 162};

constexpr int CAT_X = 120;
constexpr int FLOOR_Y = STAGE_H - 14;  // inside the stage sprite

constexpr uint16_t BG        = pal::paper;
constexpr uint16_t HEADER_BG = rgb(62, 52, 84);
constexpr uint16_t BAR_BG    = rgb(230, 222, 210);
constexpr uint16_t WALL_DAY  = rgb(198, 228, 250);
constexpr uint16_t FLOOR_DAY = rgb(236, 214, 182);
constexpr uint16_t WALL_NIGHT  = rgb(38, 42, 86);
constexpr uint16_t FLOOR_NIGHT = rgb(66, 58, 92);

constexpr uint16_t BTN_COLOR[3] = {rgb(250, 190, 70), rgb(110, 200, 120), rgb(120, 140, 230)};

uint16_t shade(uint16_t c) {  // ~65% brightness of an RGB565 colour
  uint16_t r = (c >> 11) & 0x1F, g = (c >> 5) & 0x3F, b = c & 0x1F;
  return uint16_t(((r * 2 / 3) << 11) | ((g * 2 / 3) << 5) | (b * 2 / 3));
}

uint32_t animLength(Reaction r) {
  switch (r) {
    case Reaction::Eat: return 2200;
    case Reaction::Play: return 2000;
    case Reaction::Purr: return 1500;
    case Reaction::None: return 0;
    default: return 1600;
  }
}

float catScale(Stage s) {
  switch (s) {
    case Stage::Kitten: return 0.70f;
    case Stage::Young: return 0.86f;
    default: return 1.0f;
  }
}

const char* stageName(Stage s) {
  switch (s) {
    case Stage::Kitten: return "Kitten";
    case Stage::Young: return "Young cat";
    default: return "Adult cat";
  }
}

}  // namespace

void Ui::begin() {
  _stage.setColorDepth(16);
  if (!_stage.createSprite(W, STAGE_H)) {
    Serial.println("[ui] low memory, using 8-bit stage");
    _stage.setColorDepth(8);
    _stage.createSprite(W, STAGE_H);
  }
#if USE_RGB_LED
  for (int pin : {PIN_LED_R, PIN_LED_G, PIN_LED_B}) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);  // active low: off
  }
#endif
  _lastTouch = millis();
  redrawAll();
}

void Ui::update() {
  uint32_t now = millis();
  handleTouch();

  if (_mode == Mode::Play) {
    if (_pet.revision() != _seenRevision) {
      _seenRevision = _pet.revision();
      drawHeader(false);
      drawStats(false);
      if (_pet.sleeping() != _shownSleeping) drawButtons();
    }
    if (_pressedBtn >= 0 && now > _pressedUntil) {
      drawButton(_pressedBtn, false);
      _pressedBtn = -1;
    }
    uint32_t frameMs = _brightness >= BRIGHTNESS_ACTIVE || _anim != Reaction::None ? 33 : 100;
    if (now - _lastFrame >= frameMs) {
      _lastFrame = now;
      renderStage();
    }
  }

  updateBacklight();
  updateLed();
}

// --------------------------------------------------------------------------
// Static parts
// --------------------------------------------------------------------------

void Ui::redrawAll() {
  _lcd.fillScreen(BG);
  _shownHeader[0] = 0;
  for (auto& p : _shownPct) p = -1;
  drawHeader(true);
  drawStats(true);
  drawButtons();
  renderStage();
}

void Ui::drawHeader(bool force) {
  char info[24];
  uint32_t age = _pet.ageMinutes();
  if (age < 60) snprintf(info, sizeof(info), "%s %lum", stageName(_pet.stage()), (unsigned long)age);
  else if (age < 48 * 60) snprintf(info, sizeof(info), "%s %luh", stageName(_pet.stage()), (unsigned long)(age / 60));
  else snprintf(info, sizeof(info), "%s %lud", stageName(_pet.stage()), (unsigned long)(age / 1440));
  if (!force && strcmp(info, _shownHeader) == 0) return;
  strlcpy(_shownHeader, info, sizeof(_shownHeader));

  _lcd.fillRect(0, 0, W, HEADER_H, HEADER_BG);
  drawHeart(_lcd, 14, 14, 6, pal::pink);
  _lcd.setTextColor(pal::white);
  _lcd.setFont(&fonts::FreeSansBold12pt7b);
  _lcd.setTextDatum(textdatum_t::middle_left);
  _lcd.drawString("DigiCat", 26, HEADER_H / 2 + 1);
  _lcd.setFont(&fonts::Font2);
  _lcd.setTextDatum(textdatum_t::middle_right);
  _lcd.setTextColor(rgb(210, 200, 235));
  _lcd.drawString(info, W - 8, HEADER_H / 2 + 1);
}

void Ui::drawStats(bool force) {
  const PetStats& st = _pet.stats();
  const int16_t values[4] = {st.food, st.fun, st.energy, st.health};
  for (int i = 0; i < 4; i++) {
    int8_t pct = values[i] / 10;
    if (!force && pct == _shownPct[i]) continue;
    _shownPct[i] = pct;

    int x = (i % 2) ? 122 : 6, y = STATS_Y + (i / 2) * 32;
    _lcd.fillRect(x, y, 112, 28, BG);
    int ix = x + 11, iy = y + 13;
    switch (i) {
      case 0: drawFish(_lcd, ix, iy, 10, rgb(240, 140, 60)); break;
      case 1: drawBall(_lcd, ix, iy, 8, pal::red); break;
      case 2: drawBolt(_lcd, ix, iy, 10, pal::yellow); break;
      case 3: drawHeart(_lcd, ix, iy - 2, 7, pal::pink); break;
    }
    int bx = x + 26, bw = 84, by = y + 7, bh = 13;
    uint16_t c = pct > 50 ? pal::green : pct > 25 ? pal::yellow : pal::red;
    _lcd.fillRoundRect(bx, by, bw, bh, 6, BAR_BG);
    int fw = max(pct > 0 ? 8 : 0, bw * pct / 100);
    if (fw) _lcd.fillRoundRect(bx, by, fw, bh, 6, c);
  }
}

void Ui::drawButtons() {
  _shownSleeping = _pet.sleeping();
  for (int i = 0; i < 3; i++) drawButton(i, _pressedBtn == i);
}

void Ui::drawButton(int i, bool pressed) {
  const int x = BTN_X[i], depth = 4, off = pressed ? depth : 0;
  const uint16_t c = BTN_COLOR[i];
  _lcd.fillRect(x, BTN_Y, BTN_W, BTN_H + depth, BG);
  _lcd.fillRoundRect(x, BTN_Y + depth, BTN_W, BTN_H, 10, shade(c));
  _lcd.fillRoundRect(x, BTN_Y + off, BTN_W, BTN_H, 10, c);

  const int ix = x + 16, iy = BTN_Y + off + BTN_H / 2;
  const char* label = "FEED";
  switch (i) {
    case 0: drawFish(_lcd, ix, iy, 11, pal::white); break;
    case 1: drawBall(_lcd, ix, iy, 8, pal::white); label = "PLAY"; break;
    case 2:
      drawMoon(_lcd, ix, iy, 8, pal::white, c);
      label = _pet.sleeping() ? "WAKE" : "SLEEP";
      break;
  }
  _lcd.setFont(&fonts::Font2);
  _lcd.setTextColor(pal::ink);
  _lcd.setTextDatum(textdatum_t::middle_center);
  _lcd.drawString(label, x + 46, iy + 1);
}

void Ui::drawConfirm() {
  _lcd.fillScreen(HEADER_BG);
  drawCatSitting(_lcd, W / 2, 150, CatPose{});
  _lcd.setTextColor(pal::white);
  _lcd.setTextDatum(textdatum_t::middle_center);
  _lcd.setFont(&fonts::FreeSansBold12pt7b);
  _lcd.drawString("Start over?", W / 2, 186);
  _lcd.setFont(&fonts::Font2);
  _lcd.drawString("Your cat will be a kitten again.", W / 2, 214);
  _lcd.fillRoundRect(16, 250, 96, 48, 10, pal::red);
  _lcd.fillRoundRect(128, 250, 96, 48, 10, pal::green);
  _lcd.setFont(&fonts::FreeSansBold9pt7b);
  _lcd.setTextColor(pal::white);
  _lcd.drawString("RESET", 64, 274);
  _lcd.drawString("KEEP", 176, 274);
}

// --------------------------------------------------------------------------
// The animated room (double buffered in a sprite, so it never flickers)
// --------------------------------------------------------------------------

void Ui::drawRoom(bool night) {
  auto& g = _stage;
  const uint16_t wall = night ? WALL_NIGHT : WALL_DAY;
  g.fillScreen(wall);
  g.fillRect(0, FLOOR_Y, W, STAGE_H - FLOOR_Y, night ? FLOOR_NIGHT : FLOOR_DAY);
  g.drawFastHLine(0, FLOOR_Y, W, night ? shade(FLOOR_NIGHT) : shade(FLOOR_DAY));

  // Window, top left
  const int wx = 18, wy = 14, ww = 50, wh = 42;
  const uint16_t frame = night ? rgb(90, 90, 140) : pal::white;
  const uint16_t sky = night ? rgb(16, 18, 50) : rgb(130, 200, 255);
  g.fillRoundRect(wx - 3, wy - 3, ww + 6, wh + 6, 4, frame);
  g.fillRect(wx, wy, ww, wh, sky);
  if (night) {
    drawMoon(g, wx + 32, wy + 14, 8, rgb(250, 240, 180), sky);
    g.drawPixel(wx + 10, wy + 8, pal::white);
    g.drawPixel(wx + 18, wy + 28, pal::white);
    g.drawPixel(wx + 40, wy + 34, pal::white);
  } else {
    g.fillCircle(wx + 38, wy + 12, 7, pal::yellow);
    g.fillEllipse(wx + 16, wy + 30, 10, 5, pal::white);
    g.fillEllipse(wx + 24, wy + 27, 8, 5, pal::white);
  }
  g.drawFastVLine(wx + ww / 2, wy, wh, frame);
  g.drawFastHLine(wx, wy + wh / 2, ww, frame);

  // Cushion under the cat
  g.fillEllipse(CAT_X, FLOOR_Y + 2, 56, 8, night ? rgb(110, 70, 120) : rgb(230, 120, 140));
}

void Ui::renderStage() {
  const uint32_t now = millis();
  if (_anim != Reaction::None && now - _animStart >= _animLen) _anim = Reaction::None;
  const float t = _anim == Reaction::None ? 0 : float(now - _animStart) / _animLen;  // 0..1

  const bool asleep = _pet.sleeping();
  drawRoom(asleep);

  CatPose pose;
  pose.scale = catScale(_pet.stage());
  const Mood mood = _pet.mood();
  if (mood == Mood::Sick) {
    pose.fur = pal::furSick;
    pose.stripe = pal::stripeSick;
  }
  pose.breathe = uint8_t((sinf(now * 0.004f) + 1) * (asleep ? 1.5f : 1.0f));
  pose.tail = sinf(now * 0.0025f);

  // Idle life: blinking and looking around
  if (now >= _nextBlink) {
    _blinkUntil = now + 140;
    _nextBlink = now + random(2500, 6500);
  }
  if (now >= _nextLook) {
    _look = random(-3, 4);
    _nextLook = now + random(3000, 9000);
  }
  pose.look = _look;

  switch (mood) {
    case Mood::Happy: pose.mouth = CatPose::Mouth::Smile; pose.blush = true; break;
    case Mood::Ok: pose.mouth = CatPose::Mouth::Smile; break;
    case Mood::Sad: pose.eyes = CatPose::Eyes::Sad; pose.mouth = CatPose::Mouth::Frown; break;
    case Mood::Sick: pose.eyes = CatPose::Eyes::Sad; pose.mouth = CatPose::Mouth::Neutral; break;
    case Mood::Sleeping: break;
  }
  if (now < _blinkUntil && pose.eyes != CatPose::Eyes::Happy) pose.eyes = CatPose::Eyes::Closed;

  int jump = 0;
  const int headTop = FLOOR_Y - int(100 * pose.scale);

  switch (_anim) {
    case Reaction::Eat: {
      int fill = 3 - int(t * 3.99f);
      drawBowl(_stage, CAT_X - 66, FLOOR_Y + 2, fill);
      pose.look = -3;
      pose.eyes = t > 0.8f ? CatPose::Eyes::Happy : CatPose::Eyes::Open;
      pose.mouth = ((now / 180) % 2 && t < 0.8f) ? CatPose::Mouth::Open : CatPose::Mouth::Smile;
      break;
    }
    case Reaction::Play: {
      float hop = fabsf(sinf(t * 3 * PI));
      jump = int(hop * 24);
      int bx = CAT_X + 80 - int(t * 160);
      int by = FLOOR_Y - 8 - int(fabsf(cosf(t * 4 * PI)) * 40);
      drawBall(_stage, bx, by, 7, pal::red);
      pose.eyes = CatPose::Eyes::Happy;
      pose.mouth = CatPose::Mouth::Open;
      pose.look = bx < CAT_X ? -3 : 3;
      break;
    }
    case Reaction::Purr:
      if (!asleep) {
        pose.eyes = CatPose::Eyes::Happy;
        pose.blush = true;
      }
      for (int i = 0; i < 3; i++) {
        float p = t * 1.4f - i * 0.2f;
        if (p < 0 || p > 1) continue;
        int hx = CAT_X - 20 + i * 20 + int(sinf(p * 6 + i) * 6);
        drawHeart(_stage, hx, headTop + 10 - int(p * 40), 5 + i % 2, pal::red);
      }
      break;
    case Reaction::NotHungry:
    case Reaction::TooTired:
      pose.look = (now / 150) % 2 ? -3 : 3;  // shakes head
      pose.mouth = CatPose::Mouth::Neutral;
      break;
    default:
      break;
  }

  if (jump) _stage.fillEllipse(CAT_X, FLOOR_Y + 2, 26 - jump / 2, 4, shade(WALL_DAY));

  if (asleep) {
    drawCatSleeping(_stage, CAT_X, FLOOR_Y, pose);
    // Floating "z z Z"
    _stage.setTextColor(pal::white);
    _stage.setTextDatum(textdatum_t::middle_center);
    for (int i = 0; i < 3; i++) {
      float p = fmodf(now * 0.0004f + i / 3.0f, 1.0f);
      if (p > 0.6f) _stage.setFont(&fonts::FreeSansBold9pt7b);
      else _stage.setFont(&fonts::Font2);
      _stage.drawString("z", CAT_X - 30 + int(p * 40), FLOOR_Y - 50 - int(p * 50));
    }
  } else {
    drawCatSitting(_stage, CAT_X, FLOOR_Y - jump, pose);
  }

  // Speech bubble for reactions and for needs while idle
  Reaction bubble = Reaction::None;
  if (_anim == Reaction::NotHungry || _anim == Reaction::TooTired || _anim == Reaction::WokeUp) {
    bubble = _anim;
  } else if (_anim == Reaction::None && !asleep && (now / 4000) % 2) {
    const PetStats& st = _pet.stats();
    if (st.food < 250) bubble = Reaction::Eat;
    else if (st.energy < 200) bubble = Reaction::FellAsleep;
    else if (st.fun < 250) bubble = Reaction::Play;
  }
  if (bubble != Reaction::None) drawBubble(CAT_X + int(40 * pose.scale), max(8, headTop - 8), bubble);

  _stage.pushSprite(&_lcd, 0, STAGE_Y);
}

void Ui::drawBubble(int x, int y, Reaction what) {
  auto& g = _stage;
  g.fillRoundRect(x, y, 40, 30, 10, pal::white);
  g.fillTriangle(x + 6, y + 24, x + 16, y + 28, x - 2, y + 38, pal::white);
  const int cx = x + 20, cy = y + 15;
  switch (what) {
    case Reaction::Eat: drawFish(g, cx, cy, 12, rgb(240, 140, 60)); break;
    case Reaction::Play: drawBall(g, cx, cy, 8, pal::red); break;
    case Reaction::FellAsleep: drawMoon(g, cx, cy, 9, pal::blue, pal::white); break;
    case Reaction::NotHungry:
      drawFish(g, cx, cy, 12, rgb(240, 140, 60));
      g.drawWideLine(cx - 10, cy - 9, cx + 10, cy + 9, 1.5f, pal::red);
      break;
    case Reaction::TooTired:
      drawBolt(g, cx, cy, 10, rgb(190, 190, 190));
      break;
    case Reaction::WokeUp:
      g.setFont(&fonts::FreeSansBold12pt7b);
      g.setTextColor(pal::ink);
      g.setTextDatum(textdatum_t::middle_center);
      g.drawString("!", cx, cy);
      break;
    default: break;
  }
}

// --------------------------------------------------------------------------
// Touch
// --------------------------------------------------------------------------

Ui::Hit Ui::hitTest(int x, int y) const {
  if (_mode == Mode::ConfirmReset) {
    if (y >= 240) return x < W / 2 ? Hit::Yes : Hit::No;
    return Hit::None;
  }
  if (y < HEADER_H) return Hit::Header;
  if (y >= BTN_Y - 4) {
    for (int i = 0; i < 3; i++)
      if (x >= BTN_X[i] - 3 && x < BTN_X[i] + BTN_W + 3) return Hit(i);
    return Hit::None;
  }
  if (y >= STAGE_Y && y < STAGE_Y + STAGE_H && abs(x - CAT_X) < 60) return Hit::Cat;
  return Hit::None;
}

void Ui::handleTouch() {
  int32_t x, y;
  const bool down = _lcd.getTouch(&x, &y);
  const uint32_t now = millis();

  if (down && !_touching) {  // finger down
    _touching = true;
    _downAt = now;
    // A dimmed screen only wakes up on the first touch
    _swallow = _brightness < BRIGHTNESS_ACTIVE && now - _lastTouch > 10000;
    _lastTouch = now;
    _downHit = _swallow ? Hit::None : hitTest(x, y);
    if (_downHit != Hit::None && _downHit != Hit::Header) onPress(_downHit);
  } else if (down) {  // still holding
    _lastTouch = now;
    if (_downHit == Hit::Cat && hitTest(x, y) == Hit::Cat) react(_pet.pet());  // stroking keeps purring
    if (_downHit == Hit::Header && _mode == Mode::Play) {
      int w = min<uint32_t>(W, (now - _downAt) * W / RESET_HOLD_MS);
      if (w > _holdDrawn) {  // progress bar while holding the title
        _lcd.fillRect(_holdDrawn, HEADER_H - 3, w - _holdDrawn, 3, pal::red);
        _holdDrawn = w;
      }
      if (now - _downAt >= RESET_HOLD_MS) {
        _downHit = Hit::None;
        _mode = Mode::ConfirmReset;
        drawConfirm();
      }
    }
  } else if (_touching) {  // finger up
    _touching = false;
    if (_holdDrawn) {
      _holdDrawn = 0;
      if (_mode == Mode::Play) drawHeader(true);
    }
  }
}

void Ui::onPress(Hit hit) {
  switch (hit) {
    case Hit::Feed:
    case Hit::PlayBtn:
    case Hit::Sleep: {
      int i = int(hit);
      _pressedBtn = i;
      _pressedUntil = millis() + 150;
      drawButton(i, true);
      react(i == 0 ? _pet.feed() : i == 1 ? _pet.play() : _pet.toggleSleep());
      break;
    }
    case Hit::Cat:
      react(_pet.pet());
      break;
    case Hit::Yes:
    case Hit::No:
      if (hit == Hit::Yes) _pet.reset();
      _mode = Mode::Play;
      _anim = Reaction::None;
      redrawAll();
      break;
    default:
      break;
  }
}

void Ui::react(Reaction r) {
  if (r == Reaction::None || r == Reaction::FellAsleep) {
    _anim = Reaction::None;
    return;
  }
  if (r == Reaction::Snore) r = Reaction::Purr;    // sleeping cat: just some hearts
  if (r == _anim && r == Reaction::Purr) return;  // keep the running heart animation
  _anim = r;
  _animStart = millis();
  _animLen = animLength(r);
}

// --------------------------------------------------------------------------
// Backlight and LED
// --------------------------------------------------------------------------

void Ui::updateBacklight() {
  const uint32_t idle = millis() - _lastTouch;
  uint8_t target;
  if (_pet.sleeping()) target = idle < 10000 ? BRIGHTNESS_IDLE : BRIGHTNESS_NIGHT;
  else target = idle < IDLE_DIM_MS ? BRIGHTNESS_ACTIVE : BRIGHTNESS_IDLE;
  if (_mode == Mode::ConfirmReset) target = BRIGHTNESS_ACTIVE;
  if (target == _brightness) return;
  // Fade gently down (~1.5 s), jump straight up
  if (target < _brightness && millis() - _lastFade < 8) return;
  _lastFade = millis();
  _brightness = target > _brightness ? target : _brightness - 1;
  _lcd.setBrightness(_brightness);
}

void Ui::updateLed() {
#if USE_RGB_LED
  const bool on = _pet.needsAttention() && (millis() % 3000) < 80;
  digitalWrite(PIN_LED_R, on ? LOW : HIGH);
#endif
}
