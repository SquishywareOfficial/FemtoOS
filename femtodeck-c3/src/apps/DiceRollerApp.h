#pragma once

#include "../../App.h"

class DiceRollerApp : public App {
  public:
    DiceRollerApp(uint32_t width, uint32_t height, uint32_t left = 1);

  protected:
    void onAppReset() override;
    void updateRunning(uint32_t deltaMs, const ButtonInput& input) override;
    void drawRunning(U8G2& u8g2) override;
    bool startsRunningImmediately() const override;
    bool hasCustomOverlay() const override;

  private:
    enum class Mode { Select, Roll };

    static const uint8_t DICE[];
    static const uint8_t DICE_COUNT;

    Mode mode_ = Mode::Select;
    uint8_t selected_ = 1;
    uint8_t result_ = 1;
    uint16_t animMs_ = 0;
    bool rolling_ = false;
};
