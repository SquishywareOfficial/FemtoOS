#include <U8g2lib.h>
#include <Wire.h>

#include "BlackjackGame.h"
#include "BreakoutGame.h"
#include "CreditsGame.h"
#include "DefenderMiniGame.h"
#include "FishingFlickGame.h"
#include "Game.h"
#include "HeliCaveGame.h"
#include "InitialsGame.h"
#include "JumpGame.h"
#include "MazeRunnerGame.h"
#include "MiniLanderGame.h"
#include "MicroRacerGame.h"
#include "NeedSpeedGame.h"
#include "NoonShooterGame.h"
#include "PipeManiaGame.h"
#include "PlayerProfile.h"
#include "TinyGolfGame.h"
#include "TowerStackerGame.h"

class U8G2_SSD1306_72X40_NONAME_F_HW_I2C : public U8G2 {
  public:
    U8G2_SSD1306_72X40_NONAME_F_HW_I2C(
        const u8g2_cb_t* rotation, uint8_t reset = U8X8_PIN_NONE, uint8_t clock = U8X8_PIN_NONE, uint8_t data = U8X8_PIN_NONE)
        : U8G2() {
      u8g2_Setup_ssd1306_i2c_72x40_er_f(&u8g2, rotation, u8x8_byte_arduino_hw_i2c, u8x8_gpio_and_delay_arduino);
      u8x8_SetPin_HW_I2C(getU8x8(), reset, clock, data);
    }
};

U8G2_SSD1306_72X40_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6, 5);

constexpr uint8_t BUTTON_PIN = 9;
constexpr uint8_t SCREEN_WIDTH = 72;
constexpr uint8_t SCREEN_HEIGHT = 40;
constexpr uint8_t GAME_WIDTH = 70;
constexpr uint8_t GAME_HEIGHT = 40;
constexpr uint8_t GAME_LEFT = 1;

bool isButtonDown() {
  return digitalRead(BUTTON_PIN) == LOW;
}

BreakoutGame breakoutGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
MicroRacerGame microRacerGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
DefenderMiniGame defenderMiniGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
JumpGame jumpGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
HeliCaveGame heliCaveGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
MiniLanderGame miniLanderGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
NeedSpeedGame needSpeedGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
NoonShooterGame noonShooterGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
FishingFlickGame fishingFlickGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
MazeRunnerGame mazeRunnerGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
MazeRunnerGame mazeCollectorGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT, true);
PipeManiaGame pipeManiaGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
BlackjackGame blackjackGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
TinyGolfGame tinyGolfGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
TowerStackerGame towerStackerGame(GAME_WIDTH, GAME_HEIGHT, GAME_LEFT);
InitialsGame initialsGame(GAME_WIDTH, GAME_HEIGHT);
CreditsGame creditsGame(GAME_WIDTH, GAME_HEIGHT);

Game* games[] = {
    &miniLanderGame,
    &needSpeedGame,
    &noonShooterGame,
    &fishingFlickGame,
    &mazeRunnerGame,
    &mazeCollectorGame,
    &pipeManiaGame,
    &blackjackGame,
    &tinyGolfGame,
    &towerStackerGame,
    &initialsGame,
    &creditsGame,
    &breakoutGame,
    &microRacerGame,
    &defenderMiniGame,
    &jumpGame,
    &heliCaveGame
  };
constexpr uint8_t GAME_COUNT = sizeof(games) / sizeof(games[0]);

SingleButton menuButton;
uint8_t menuIndex = 0;
bool inMenu = true;
bool menuLaunchArmed = false;
Game* activeGame = nullptr;

void drawMenu() {
  u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(3, 9, "Select game");
  if (strlen(games[menuIndex]->gameTitle()) > 12) {
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.drawStr(3, 19, games[menuIndex]->gameTitle());
    u8g2.setFont(u8g2_font_5x8_tr);
  } else {
    u8g2.drawStr(3, 19, games[menuIndex]->gameTitle());
  }
  if (menuLaunchArmed) {
    u8g2.drawStr(3, 29, "Release");
    u8g2.drawStr(3, 38, "to start");
  } else {
    u8g2.drawStr(3, 29, "Tap next");
    u8g2.drawStr(3, 38, "Hold start");
  }
}

void drawGameOverlay(Game& game) {
  u8g2.drawFrame(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  u8g2.setFont(u8g2_font_5x8_tr);
  if (game.phase() == GamePhase::Start) {
    if (game.showStartPromptPage()) {
      u8g2.drawStr(20, 16, "Press");
      u8g2.drawStr(13, 29, "to Start");
    } else if (game.showStartScorePage()) {
      u8g2.drawStr(3, 10, "Top Score");
      u8g2.setFont(u8g2_font_4x6_tr);
      u8g2.drawStr(3, 24, "--");
    } else {
      u8g2.drawStr(3, 10, game.gameTitle());
    }
  } else if (game.phase() == GamePhase::End) {
    u8g2.drawStr(3, 10, "Game Over");
    u8g2.drawStr(3, 24, "Tap retry");
    u8g2.drawStr(3, 36, "Hold menu");
  }
}

void setup(void) {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  u8g2.begin();
  u8g2.setContrast(255);
  u8g2.setBusClock(400000);
  randomSeed(micros());
  menuButton.reset(isButtonDown(), millis());
  menuLaunchArmed = false;
}

void loop(void) {
  const uint32_t nowMs = millis();
  const bool buttonDown = isButtonDown();

  u8g2.clearBuffer();

  if (inMenu) {
    const ButtonInput input = menuButton.update(buttonDown, nowMs);
    if (input.longPress) {
      menuLaunchArmed = true;
    }
    if (input.click && !menuLaunchArmed) {
      menuIndex = (menuIndex + 1) % GAME_COUNT;
    }
    if (menuLaunchArmed && input.released) {
      activeGame = games[menuIndex];
      activeGame->begin(nowMs, buttonDown);
      inMenu = false;
      menuLaunchArmed = false;
    }
    drawMenu();
  } else {
    activeGame->tick(nowMs, buttonDown);
    activeGame->render(u8g2);
    if (activeGame->phase() != GamePhase::Running && !activeGame->hasCustomOverlay()) {
      drawGameOverlay(*activeGame);
    }
    if (activeGame->shouldExitToMenu()) {
      activeGame->clearExitRequest();
      menuButton.reset(buttonDown, nowMs);
      inMenu = true;
      menuLaunchArmed = false;
    }
  }

  u8g2.sendBuffer();
}
