#include "DiceRollerApp.h"

#include <Arduino.h>
#include <U8g2lib.h>

const uint8_t DiceRollerApp::DICE[] = {4, 6, 8, 10, 12, 20};
const uint8_t DiceRollerApp::DICE_COUNT = sizeof(DiceRollerApp::DICE) / sizeof(DiceRollerApp::DICE[0]);

DiceRollerApp::DiceRollerApp(uint32_t width, uint32_t height, uint32_t left)
    : App("Dice Roller", width, height) {
  (void)left;
}

bool DiceRollerApp::startsRunningImmediately() const {
  return true;
}

bool DiceRollerApp::hasCustomOverlay() const {
  return true;
}

void DiceRollerApp::onAppReset() {
  mode_ = Mode::Select;
  selected_ = 1;
  result_ = 1;
  animMs_ = 0;
  rolling_ = false;
}

void DiceRollerApp::updateRunning(uint32_t deltaMs, const ButtonInput& input) {
  if (mode_ == Mode::Select) {
    if (input.click) {
      selected_ = (selected_ + 1) % (DICE_COUNT + 1);
    }
    if (input.longPress) {
      if (selected_ >= DICE_COUNT) {
        requestExitToMenu();
      } else {
        mode_ = Mode::Roll;
        result_ = static_cast<uint8_t>(random(1, DICE[selected_] + 1));
        animMs_ = 0;
        rolling_ = true;
      }
    }
    return;
  }

  if (input.longPress) {
    mode_ = Mode::Select;
    rolling_ = false;
    return;
  }
  if (input.click) {
    result_ = static_cast<uint8_t>(random(1, DICE[selected_] + 1));
    animMs_ = 0;
    rolling_ = true;
  }
  if (rolling_) {
    animMs_ += deltaMs;
    if (animMs_ > 450) {
      rolling_ = false;
    } else if ((animMs_ % 80) < 35) {
      result_ = static_cast<uint8_t>(random(1, DICE[selected_] + 1));
    }
  }
}

void DiceRollerApp::drawRunning(U8G2& u8g2) {
  u8g2.drawFrame(0, 0, width + 2, height);
  u8g2.setFont(u8g2_font_4x6_tr);

  if (mode_ == Mode::Select) {
    u8g2.drawStr(3, 8, "Select die");
    u8g2.setFont(u8g2_font_7x13_tr);
    if (selected_ >= DICE_COUNT) {
      u8g2.drawStr(22, 24, "Exit");
    } else {
      char die[8];
      snprintf(die, sizeof(die), "d%u", DICE[selected_]);
      const int x = (width + 2 - u8g2.getStrWidth(die)) / 2;
      u8g2.drawStr(x, 25, die);
    }
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(3, 37, "Tap next Hold ok");
    return;
  }

  char die[8];
  snprintf(die, sizeof(die), "d%u", DICE[selected_]);
  u8g2.drawStr(3, 8, die);
  u8g2.drawRFrame(22, 10, 28, 22, 3);
  u8g2.setFont(u8g2_font_7x13B_tr);
  char value[8];
  snprintf(value, sizeof(value), "%u", result_);
  u8g2.drawStr(36 - (u8g2.getStrWidth(value) / 2), 26, value);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(3, 37, "Tap roll Hold dice");
}
