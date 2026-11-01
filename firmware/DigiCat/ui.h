// DigiCat - screen layout, animations and touch handling
#pragma once
#include "display.h"
#include "pet.h"

class Ui {
 public:
  Ui(LGFX& lcd, Pet& pet) : _lcd(lcd), _pet(pet), _stage(&lcd) {}
  void begin();
  void update();  // call every loop

 private:
  enum class Mode : uint8_t { Play, ConfirmReset };
  enum class Hit : int8_t { None = -1, Feed, PlayBtn, Sleep, Cat, Header, Yes, No };

  void redrawAll();
  void drawHeader(bool force);
  void drawStats(bool force);
  void drawButtons();
  void drawButton(int i, bool pressed);
  void drawConfirm();
  void renderStage();
  void drawRoom(bool night);
  void drawBubble(int x, int y, Reaction what);

  void handleTouch();
  Hit  hitTest(int x, int y) const;
  void onPress(Hit hit);
  void react(Reaction r);
  void updateBacklight();
  void updateLed();

  LGFX&       _lcd;
  Pet&        _pet;
  LGFX_Sprite _stage;

  Mode     _mode = Mode::Play;
  uint32_t _seenRevision = 0;
  int8_t   _shownPct[4] = {-1, -1, -1, -1};
  char     _shownHeader[24] = "";
  bool     _shownSleeping = false;

  // touch
  bool     _touching = false;
  bool     _swallow = false;
  Hit      _downHit = Hit::None;
  uint32_t _downAt = 0;
  uint32_t _lastTouch = 0;
  int8_t   _pressedBtn = -1;
  uint32_t _pressedUntil = 0;
  int      _holdDrawn = 0;

  // animation
  Reaction _anim = Reaction::None;
  uint32_t _animStart = 0;
  uint32_t _animLen = 0;
  uint32_t _lastFrame = 0;
  uint32_t _nextBlink = 0;
  uint32_t _blinkUntil = 0;
  uint32_t _nextLook = 0;
  int8_t   _look = 0;

  uint8_t  _brightness = 0;
  uint32_t _lastFade = 0;
};
