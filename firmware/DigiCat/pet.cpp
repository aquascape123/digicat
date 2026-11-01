#include "pet.h"
#include <Preferences.h>
#include "config.h"

namespace {
constexpr int16_t kMax = 1000;

Preferences prefs;

void add(int16_t& v, int delta) { v = constrain(v + delta, 0, kMax); }
}  // namespace

void Pet::begin() {
  prefs.begin("digicat", false);
  bool loaded = prefs.getBytesLength("pet") == sizeof(Saved) &&
                prefs.getBytes("pet", &_s, sizeof(Saved)) == sizeof(Saved) &&
                _s.version == kVersion;
  if (!loaded) {
    reset();
  } else {
    Serial.printf("[pet] restored: age %lu min\n", (unsigned long)_s.ageMinutes);
  }
  _lastMinute = millis();
  _lastSave = millis();
}

void Pet::reset() {
  _s = Saved{};
  _s.version = kVersion;
  _s.stats = {700, 600, 800, 900};
  changed(true);
  save();
  Serial.println("[pet] new kitten");
}

void Pet::update() {
  uint32_t now = millis();
  while (now - _lastMinute >= SIM_MINUTE_MS) {
    _lastMinute += SIM_MINUTE_MS;
    simulateMinute();
  }
}

void Pet::simulateMinute() {
  PetStats& st = _s.stats;
  _s.ageMinutes++;

  if (_s.sleeping) {
    add(st.food, -FOOD_DECAY_ASLEEP);
    add(st.fun, -FUN_DECAY_ASLEEP);
    add(st.energy, ENERGY_GAIN_ASLEEP);
    if (st.energy >= kMax) _s.sleeping = false;  // well rested, wakes up by itself
  } else {
    add(st.food, -FOOD_DECAY_AWAKE);
    add(st.fun, -FUN_DECAY_AWAKE);
    add(st.energy, -ENERGY_DECAY_AWAKE);
    if (st.energy <= 30) _s.sleeping = true;      // falls asleep from exhaustion
  }

  bool critical = st.food < 200 || st.fun < 150 || st.energy < 100;
  bool fine = st.food > 500 && st.fun > 500 && st.energy > 400;
  if (critical) add(st.health, -HEALTH_LOSS);
  else if (fine) add(st.health, HEALTH_GAIN);

  changed(false);
}

Reaction Pet::feed() {
  if (_s.sleeping) return Reaction::Snore;
  if (_s.stats.food > 900) return Reaction::NotHungry;
  add(_s.stats.food, FEED_FOOD);
  add(_s.stats.energy, FEED_ENERGY);
  changed(true);
  return Reaction::Eat;
}

Reaction Pet::play() {
  if (_s.sleeping) return Reaction::Snore;
  if (_s.stats.energy < PLAY_ENERGY + 30) return Reaction::TooTired;
  add(_s.stats.fun, PLAY_FUN);
  add(_s.stats.energy, -PLAY_ENERGY);
  add(_s.stats.food, -PLAY_FOOD);
  changed(true);
  return Reaction::Play;
}

Reaction Pet::pet() {
  if (_s.sleeping) return Reaction::Snore;
  uint32_t now = millis();
  if (now - _lastPet < PET_COOLDOWN_MS) return Reaction::Purr;  // still purring, no extra bonus
  _lastPet = now;
  add(_s.stats.fun, PET_FUN);
  changed(true);
  return Reaction::Purr;
}

Reaction Pet::toggleSleep() {
  _s.sleeping = !_s.sleeping;
  changed(true);
  return _s.sleeping ? Reaction::FellAsleep : Reaction::WokeUp;
}

Stage Pet::stage() const {
  if (_s.ageMinutes < AGE_YOUNG_MIN) return Stage::Kitten;
  if (_s.ageMinutes < AGE_ADULT_MIN) return Stage::Young;
  return Stage::Adult;
}

Mood Pet::mood() const {
  const PetStats& st = _s.stats;
  if (_s.sleeping) return Mood::Sleeping;
  if (st.health < 300) return Mood::Sick;
  int avg = (st.food + st.fun + st.energy) / 3;
  if (avg > 650 && st.fun > 500) return Mood::Happy;
  if (avg < 350 || st.fun < 250) return Mood::Sad;
  return Mood::Ok;
}

bool Pet::needsAttention() const {
  const PetStats& st = _s.stats;
  return st.food < 200 || st.fun < 200 || st.health < 300 || (!_s.sleeping && st.energy < 150);
}

void Pet::changed(bool important) {
  _revision++;
  _dirty = true;
  if (important) _saveSoon = true;
}

void Pet::saveIfDue() {
  if (!_dirty) return;
  uint32_t since = millis() - _lastSave;
  if (since >= SAVE_INTERVAL_MS || (_saveSoon && since >= SAVE_AFTER_ACTION_MS)) save();
}

void Pet::save() {
  prefs.putBytes("pet", &_s, sizeof(Saved));
  _lastSave = millis();
  _dirty = false;
  _saveSoon = false;
}
