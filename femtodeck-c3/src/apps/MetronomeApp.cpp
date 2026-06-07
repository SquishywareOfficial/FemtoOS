#include "MetronomeApp.h"

#include <Arduino.h>
#include <U8g2lib.h>

namespace {
constexpr uint8_t METRO_LED_PIN = 8;
constexpr bool METRO_LED_ACTIVE_LOW = true;
constexpr uint16_t DOUBLE_TAP_MS = 450;
constexpr uint16_t TAP_HOLD_DOWN_MS = 900;
constexpr uint16_t HOLD_ADJUST_START_MS = 320;
constexpr uint16_t HOLD_ADJUST_STEP_MS = 180;
constexpr uint16_t EXIT_HOLD_MS = 12000;
}

MetronomeApp::MetronomeApp(uint32_t width, uint32_t height, uint32_t left)
    : App("Metronome", width, height) {
  (void)left;
}

bool MetronomeApp::startsRunningImmediately() const {
  return true;
}

bool MetronomeApp::hasCustomOverlay() const {
  return true;
}

void MetronomeApp::onAppReset() {
  pinMode(METRO_LED_PIN, OUTPUT);
  mode_ = Mode::Help;
  bpm_ = 120;
  helpPage_ = 0;
  beatMs_ = 0;
  pulseMs_ = 0;
  adjustMs_ = 0;
  clockMs_ = 0;
  lastTapMs_ = 0;
  running_ = false;
  pulseOn_ = false;
  holdMeansDown_ = false;
  setLed(false);
}

void MetronomeApp::setLed(bool on) {
  digitalWrite(METRO_LED_PIN, METRO_LED_ACTIVE_LOW ? !on : on);
}

void MetronomeApp::changeBpm(int delta) {
  int next = static_cast<int>(bpm_) + delta;
  if (next < 30) next = 30;
  if (next > 240) next = 240;
  bpm_ = static_cast<uint16_t>(next);
}

void MetronomeApp::updateRunning(uint32_t deltaMs, const ButtonInput& input) {
  clockMs_ += deltaMs;

  if (mode_ == Mode::Help) {
    if (input.click) {
      helpPage_ = (helpPage_ + 1) % 3;
    }
    if (input.longPress) {
      if (helpPage_ == 2) {
        requestExitToMenu();
      } else {
        mode_ = Mode::Bpm;
        lastTapMs_ = 0;
      }
    }
    setLed(false);
    return;
  }

  if (input.down && input.holdMs >= EXIT_HOLD_MS) {
    setLed(false);
    requestExitToMenu();
    return;
  }

  if (input.pressed) {
    holdMeansDown_ = (lastTapMs_ != 0 && (clockMs_ - lastTapMs_) <= TAP_HOLD_DOWN_MS);
    adjustMs_ = 0;
  }

  if (input.click) {
    if (lastTapMs_ != 0 && (clockMs_ - lastTapMs_) <= DOUBLE_TAP_MS) {
      running_ = !running_;
      lastTapMs_ = 0;
      beatMs_ = 0;
      pulseMs_ = 90;
      pulseOn_ = true;
    } else {
      lastTapMs_ = clockMs_;
    }
  }

  if (input.down && input.holdMs >= HOLD_ADJUST_START_MS) {
    adjustMs_ += deltaMs;
    if (adjustMs_ >= HOLD_ADJUST_STEP_MS || input.longPress) {
      adjustMs_ = 0;
      changeBpm(holdMeansDown_ ? -1 : 1);
    }
  }

  if (running_) {
    const uint16_t interval = static_cast<uint16_t>(60000UL / bpm_);
    beatMs_ += deltaMs;
    if (beatMs_ >= interval) {
      beatMs_ %= interval;
      pulseOn_ = true;
      pulseMs_ = 90;
    }
  }

  if (pulseMs_ > 0) {
    pulseMs_ = (deltaMs >= pulseMs_) ? 0 : pulseMs_ - deltaMs;
    if (pulseMs_ == 0) {
      pulseOn_ = false;
    }
  }
  setLed(pulseOn_);
}

void MetronomeApp::drawRunning(U8G2& u8g2) {
  if (mode_ == Mode::Help) {
    u8g2.drawFrame(0, 0, width + 2, height);
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(3, 9, "Metronome");
    u8g2.setFont(u8g2_font_4x6_tr);
    if (helpPage_ == 0) {
      u8g2.drawStr(3, 19, "Hold: BPM +");
      u8g2.drawStr(3, 28, "Tap then hold: -");
      u8g2.drawStr(3, 38, "Tap next Hold go");
    } else if (helpPage_ == 1) {
      u8g2.drawStr(3, 19, "Double tap:");
      u8g2.drawStr(3, 28, "start / stop");
      u8g2.drawStr(3, 38, "Tap next Hold go");
    } else {
      u8g2.drawStr(3, 19, "12s hold exits");
      u8g2.drawStr(3, 28, "from BPM screen");
      u8g2.drawStr(3, 38, "Hold menu");
    }
    return;
  }

  if (pulseOn_) {
    u8g2.drawBox(0, 0, width + 2, height);
    u8g2.setDrawColor(0);
  } else {
    u8g2.drawFrame(0, 0, width + 2, height);
  }

  char bpmText[10];
  snprintf(bpmText, sizeof(bpmText), "%u", bpm_);
  u8g2.setFont(u8g2_font_7x13B_tr);
  u8g2.drawStr((width + 2 - u8g2.getStrWidth(bpmText)) / 2, 25, bpmText);
  u8g2.setFont(u8g2_font_4x6_tr);
  if (!running_) {
    u8g2.drawStr(24, 38, "2tap");
  }

  if (pulseOn_) {
    u8g2.setDrawColor(1);
  }
}
