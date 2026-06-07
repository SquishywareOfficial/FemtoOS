#include "PetSimulatorApp.h"

#include <Arduino.h>
#include <Preferences.h>
#include <U8g2lib.h>

namespace {
Preferences petPrefs;
const char* PET_NAMES[] = {"Cat", "Dog", "Dino", "Blob"};
const char* ACTIONS[] = {"Feed", "Play", "Sleep", "Clean", "Stats", "Exit"};
constexpr uint8_t PET_COUNT = sizeof(PET_NAMES) / sizeof(PET_NAMES[0]);
constexpr uint8_t ACTION_COUNT = sizeof(ACTIONS) / sizeof(ACTIONS[0]);
}

PetSimulatorApp::PetSimulatorApp(uint32_t width, uint32_t height, uint32_t left)
    : App("Pet Simulator", width, height) {
  (void)left;
}

bool PetSimulatorApp::startsRunningImmediately() const {
  return true;
}

bool PetSimulatorApp::hasCustomOverlay() const {
  return true;
}

void PetSimulatorApp::onAppReset() {
  loadPet();
  mode_ = loaded_ ? Mode::Menu : Mode::ChoosePet;
  menuIndex_ = 0;
  careMs_ = 0;
  messageMs_ = 0;
  idleMs_ = 0;
  wanderMs_ = 0;
  petX_ = 34;
  petY_ = 21;
  petDx_ = 1;
  petDy_ = 0;
  idleMode_ = false;
}

void PetSimulatorApp::loadPet() {
  petPrefs.begin("pet", true);
  loaded_ = petPrefs.getBool("made", false);
  petType_ = petPrefs.getUChar("type", 0);
  hunger_ = petPrefs.getUChar("hung", 80);
  fun_ = petPrefs.getUChar("fun", 80);
  energy_ = petPrefs.getUChar("energy", 80);
  clean_ = petPrefs.getUChar("clean", 80);
  health_ = petPrefs.getUChar("health", 100);
  poop_ = petPrefs.getUChar("poop", 0);
  petPrefs.end();
  if (petType_ >= PET_COUNT) petType_ = 0;
}

void PetSimulatorApp::savePet() {
  petPrefs.begin("pet", false);
  petPrefs.putBool("made", true);
  petPrefs.putUChar("type", petType_);
  petPrefs.putUChar("hung", hunger_);
  petPrefs.putUChar("fun", fun_);
  petPrefs.putUChar("energy", energy_);
  petPrefs.putUChar("clean", clean_);
  petPrefs.putUChar("health", health_);
  petPrefs.putUChar("poop", poop_);
  petPrefs.end();
  loaded_ = true;
}

void PetSimulatorApp::tickCare(uint32_t deltaMs) {
  careMs_ += deltaMs;
  if (careMs_ < 5000) {
    return;
  }
  careMs_ = 0;
  if (hunger_ > 0) hunger_--;
  if (fun_ > 0) fun_--;
  if (energy_ > 0) energy_--;
  if (clean_ > 0 && poop_ > 0) clean_--;
  if (random(0, 12) == 0 && poop_ < 5) poop_++;

  const uint16_t average = (hunger_ + fun_ + energy_ + clean_) / 4;
  if (average < 25 && health_ > 0) {
    health_--;
  } else if (average > 70 && health_ < 100) {
    health_++;
  }
  savePet();
}

void PetSimulatorApp::applyAction() {
  switch (menuIndex_) {
    case 0:
      hunger_ = min<uint8_t>(100, hunger_ + 25);
      poop_ = min<uint8_t>(5, poop_ + 1);
      message_ = "Yum";
      break;
    case 1:
      fun_ = min<uint8_t>(100, fun_ + 24);
      energy_ = (energy_ > 12) ? energy_ - 12 : 0;
      clean_ = (clean_ > 8) ? clean_ - 8 : 0;
      message_ = "Happy";
      break;
    case 2:
      energy_ = min<uint8_t>(100, energy_ + 35);
      hunger_ = (hunger_ > 8) ? hunger_ - 8 : 0;
      message_ = "Zzz";
      break;
    case 3:
      clean_ = min<uint8_t>(100, clean_ + 30);
      poop_ = 0;
      message_ = "Clean";
      break;
    case 4:
      mode_ = Mode::Stats;
      return;
    case 5:
      savePet();
      requestExitToMenu();
      return;
  }
  savePet();
  mode_ = Mode::Message;
  messageMs_ = 0;
}

void PetSimulatorApp::updateRunning(uint32_t deltaMs, const ButtonInput& input) {
  tickCare(deltaMs);
  updateIdle(deltaMs);

  if (idleMode_) {
    if (input.click || input.longPress || input.pressed) {
      idleMode_ = false;
      idleMs_ = 0;
    }
    return;
  }

  if (input.click || input.longPress || input.pressed) {
    idleMs_ = 0;
  }

  if (mode_ == Mode::ChoosePet) {
    if (input.click) {
      petType_ = (petType_ + 1) % PET_COUNT;
    }
    if (input.longPress) {
      hunger_ = 80;
      fun_ = 80;
      energy_ = 80;
      clean_ = 80;
      health_ = 100;
      poop_ = 0;
      savePet();
      mode_ = Mode::Menu;
    }
    return;
  }

  if (mode_ == Mode::Stats) {
    if (input.click || input.longPress) {
      mode_ = Mode::Menu;
    }
    return;
  }

  if (mode_ == Mode::Message) {
    messageMs_ += deltaMs;
    if (messageMs_ > 900 || input.click || input.longPress) {
      mode_ = Mode::Menu;
    }
    return;
  }

  if (input.click) {
    menuIndex_ = (menuIndex_ + 1) % ACTION_COUNT;
  }
  if (input.longPress) {
    applyAction();
  }
}

void PetSimulatorApp::updateIdle(uint32_t deltaMs) {
  if (mode_ != Mode::Menu) {
    idleMs_ = 0;
    idleMode_ = false;
    return;
  }
  if (!idleMode_) {
    idleMs_ += deltaMs;
    if (idleMs_ >= 10000) {
      idleMode_ = true;
      wanderMs_ = 0;
    }
    return;
  }

  wanderMs_ += deltaMs;
  if (wanderMs_ >= 650) {
    wanderMs_ = 0;
    petDx_ = static_cast<int8_t>(random(-1, 2));
    petDy_ = static_cast<int8_t>(random(-1, 2));
  }
  petX_ = constrain(static_cast<int>(petX_) + petDx_, 9, static_cast<int>(width) - 9);
  petY_ = constrain(static_cast<int>(petY_) + petDy_, 11, static_cast<int>(height) - 8);
}

void PetSimulatorApp::drawPet(U8G2& u8g2, int x, int y) {
  if (petType_ == 0) {
    u8g2.drawCircle(x, y, 5);
    u8g2.drawLine(x - 4, y - 4, x - 6, y - 8);
    u8g2.drawLine(x + 4, y - 4, x + 6, y - 8);
  } else if (petType_ == 1) {
    u8g2.drawCircle(x, y, 5);
    u8g2.drawBox(x - 7, y - 4, 3, 5);
    u8g2.drawBox(x + 4, y - 4, 3, 5);
  } else if (petType_ == 2) {
    u8g2.drawTriangle(x - 6, y + 4, x + 7, y, x - 6, y - 5);
    u8g2.drawPixel(x + 2, y - 1);
  } else {
    u8g2.drawCircle(x, y, 6);
    u8g2.drawPixel(x - 2, y - 1);
    u8g2.drawPixel(x + 2, y - 1);
  }
  if (poop_ > 0) {
    u8g2.drawStr(x + 10, y + 5, "*");
  }
}

void PetSimulatorApp::drawBar(U8G2& u8g2, int x, int y, const char* label, uint8_t value) {
  u8g2.drawStr(x, y, label);
  u8g2.drawFrame(x + 16, y - 5, 28, 5);
  u8g2.drawBox(x + 17, y - 4, map(value, 0, 100, 0, 26), 3);
}

void PetSimulatorApp::drawRunning(U8G2& u8g2) {
  if (idleMode_) {
    u8g2.setFont(u8g2_font_4x6_tr);
    drawPet(u8g2, petX_, petY_);
    return;
  }

  u8g2.drawFrame(0, 0, width + 2, height);
  u8g2.setFont(u8g2_font_4x6_tr);

  if (mode_ == Mode::ChoosePet) {
    u8g2.drawStr(3, 8, "Choose pet");
    drawPet(u8g2, 36, 21);
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(3, 36, PET_NAMES[petType_]);
    return;
  }

  drawPet(u8g2, 17, 20);
  if (mode_ == Mode::Stats) {
    drawBar(u8g2, 31, 8, "H", hunger_);
    drawBar(u8g2, 31, 15, "F", fun_);
    drawBar(u8g2, 31, 22, "E", energy_);
    drawBar(u8g2, 31, 29, "C", clean_);
    u8g2.setCursor(31, 38);
    u8g2.print("HP");
    u8g2.print(health_);
    return;
  }

  if (mode_ == Mode::Message) {
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(34, 23, message_);
    return;
  }

  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(31, 15, PET_NAMES[petType_]);
  u8g2.drawStr(31, 28, ACTIONS[menuIndex_]);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(3, 38, "Tap next Hold");
}
