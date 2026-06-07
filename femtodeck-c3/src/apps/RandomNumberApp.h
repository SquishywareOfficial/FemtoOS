#pragma once

#include "../../App.h"

class RandomNumberApp : public App {
  public:
    RandomNumberApp(uint32_t width, uint32_t height, uint32_t left = 1);

  protected:
    void onAppReset() override;
    void updateRunning(uint32_t deltaMs, const ButtonInput& input) override;
    void drawRunning(U8G2& u8g2) override;
    bool startsRunningImmediately() const override;
    bool hasCustomOverlay() const override;

  private:
    enum class Mode { Zero, Range, Result };

    static const uint32_t RANGES[];
    static const uint8_t RANGE_COUNT;

    void roll();
    void drawNumber(U8G2& u8g2, uint32_t value, int y);

    Mode mode_ = Mode::Zero;
    bool includeZero_ = true;
    uint8_t rangeIndex_ = 0;
    uint32_t value_ = 0;
};
