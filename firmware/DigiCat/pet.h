// DigiCat - pet simulation and persistence (no drawing code here)
#pragma once
#include <Arduino.h>

enum class Stage : uint8_t { Kitten, Young, Adult };
enum class Mood : uint8_t { Happy, Ok, Sad, Sick, Sleeping };

// Outcome of a player action, used by the UI to pick an animation
enum class Reaction : uint8_t { None, Eat, NotHungry, Play, TooTired, Purr, Snore, FellAsleep, WokeUp };

struct PetStats {
  int16_t food;
  int16_t fun;
  int16_t energy;
  int16_t health;
};

class Pet {
 public:
  void begin();          // load from flash or start a new kitten
  void reset();          // brand new kitten (saved immediately)
  void update();         // call every loop; advances the simulation clock
  void saveIfDue();

  Reaction feed();
  Reaction play();
  Reaction pet();
  Reaction toggleSleep();

  const PetStats& stats() const { return _s.stats; }
  bool     sleeping() const { return _s.sleeping; }
  uint32_t ageMinutes() const { return _s.ageMinutes; }
  Stage    stage() const;
  Mood     mood() const;
  bool     needsAttention() const;
  // Bumped whenever something visible changes, so the UI can redraw lazily
  uint32_t revision() const { return _revision; }

 private:
  // Stored verbatim in NVS; bump kVersion if the layout changes
  struct Saved {
    uint8_t  version;
    uint8_t  sleeping;
    uint16_t reserved;
    uint32_t ageMinutes;
    PetStats stats;
  };
  static constexpr uint8_t kVersion = 2;

  void simulateMinute();
  void changed(bool important);
  void save();

  Saved    _s{};
  uint32_t _lastMinute = 0;
  uint32_t _lastSave = 0;
  uint32_t _lastPet = 0;
  bool     _dirty = false;
  bool     _saveSoon = false;
  uint32_t _revision = 0;
};
