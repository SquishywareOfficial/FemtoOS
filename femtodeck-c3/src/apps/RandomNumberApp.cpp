#include "RandomNumberApp.h"

#include <Arduino.h>
#include <U8g2lib.h>

const uint32_t RandomNumberApp::RANGES[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 1000, 10000, 100000, 1000000};
const uint8_t RandomNumberApp::RANGE_COUNT = sizeof(RandomNumberApp::RANGES) / sizeof(RandomNumberApp::RANGES[0]);

RandomNumberApp::RandomNumberApp(uint32_t width, uint32_t height, uint32_t left)
    : App("Random Number", width, height) {
  (void)left;
}

bool RandomNumberApp::startsRunningImmediately() const {
  return true;
}

bool RandomNumberApp::hasCustomOverlay() const {
  return true;
}

void RandomNumberApp::onAppReset() {
  mode_ = Mode::Zero;
  includeZero_ = true;
  rangeIndex_ = 0;
  value_ = 0;
}

void RandomNumberApp::roll() {
  const uint32_t low = includeZero_ ? 0 : 1;
  const uint32_t high = RANGES[rangeIndex_];
  value_ = static_cast<uint32_t>(random(static_cast<long>(low), static_cast<long>(high + 1)));
}

void RandomNumberApp::updateRunning(uint32_t deltaMs, const ButtonInput& input) {
  (void)deltaMs;
  if (mode_ == Mode::Zero) {
    if (input.click) {
      includeZero_ = !includeZero_;
    }
    if (input.longPress) {
      mode_ = Mode::Range;
    }
    return;
  }

  if (mode_ == Mode::Range) {
    if (input.click) {
      rangeIndex_ = (rangeIndex_ + 1) % (RANGE_COUNT + 1);
    }
    if (input.longPress) {
      if (rangeIndex_ >= RANGE_COUNT) {
        requestExitToMenu();
      } else {
        roll();
        mode_ = Mode::Result;
      }
    }
    return;
  }

  if (input.click) {
    roll();
  }
  if (input.longPress) {
    mode_ = Mode::Range;
  }
}

void RandomNumberApp::drawNumber(U8G2& u8g2, uint32_t value, int y) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%lu", static_cast<unsigned long>(value));
  u8g2.setFont(u8g2_font_7x13_tr);
  if (u8g2.getStrWidth(buf) > static_cast<int>(width - 4)) {
    u8g2.setFont(u8g2_font_5x8_tr);
  }
  const int x = (width + 2 - u8g2.getStrWidth(buf)) / 2;
  u8g2.drawStr(x, y, buf);
}

void RandomNumberApp::drawRunning(U8G2& u8g2) {
  u8g2.drawFrame(0, 0, width + 2, height);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(3, 8, "Random");

  if (mode_ == Mode::Zero) {
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(12, 23, includeZero_ ? "0 allowed" : "Start at 1");
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(3, 37, "Tap toggle Hold ok");
    return;
  }

  if (mode_ == Mode::Range) {
    if (rangeIndex_ >= RANGE_COUNT) {
      u8g2.setFont(u8g2_font_7x13_tr);
      u8g2.drawStr(22, 24, "Exit");
    } else {
      drawNumber(u8g2, RANGES[rangeIndex_], 24);
    }
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(3, 37, "Tap range Hold ok");
    return;
  }

  drawNumber(u8g2, value_, 24);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(3, 8);
  u8g2.print(includeZero_ ? "0-" : "1-");
  u8g2.print(RANGES[rangeIndex_]);
  u8g2.drawStr(3, 37, "Tap roll Hold range");
}
