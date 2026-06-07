#pragma once

#include "../../App.h"

class MetronomeApp : public App {
  public:
    MetronomeApp(uint32_t width, uint32_t height, uint32_t left = 1);

  protected:
    void onAppReset() override;
    void updateRunning(uint32_t deltaMs, const ButtonInput& input) override;
    void drawRunning(U8G2& u8g2) override;
    bool startsRunningImmediately() const override;
    bool hasCustomOverlay() const override;

  private:
    enum class Mode { Help, Bpm };

    void setLed(bool on);
    void changeBpm(int delta);

    Mode mode_ = Mode::Help;
    uint16_t bpm_ = 120;
    uint8_t helpPage_ = 0;
    uint16_t beatMs_ = 0;
    uint16_t pulseMs_ = 0;
    uint16_t adjustMs_ = 0;
    uint32_t clockMs_ = 0;
    uint32_t lastTapMs_ = 0;
    bool running_ = false;
    bool pulseOn_ = false;
    bool holdMeansDown_ = false;
};
