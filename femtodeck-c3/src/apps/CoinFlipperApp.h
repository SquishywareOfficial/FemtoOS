#pragma once

#include "../../App.h"

class CoinFlipperApp : public App {
  public:
    CoinFlipperApp(uint32_t width, uint32_t height, uint32_t left = 1);

  protected:
    void onAppReset() override;
    void updateRunning(uint32_t deltaMs, const ButtonInput& input) override;
    void drawRunning(U8G2& u8g2) override;
    bool startsRunningImmediately() const override;
    bool hasCustomOverlay() const override;

  private:
    enum class Mode { Ready, Flipping, Result };

    void drawCoin(U8G2& u8g2, int cx, int cy, uint8_t frame);

    Mode mode_ = Mode::Ready;
    bool heads_ = true;
    uint16_t animMs_ = 0;
};
