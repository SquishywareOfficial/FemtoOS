#include "TowerStackerGame.h"

#include <Arduino.h>
#include <Preferences.h>
#include <U8g2lib.h>

#include "PlayerProfile.h"

namespace {
Preferences towerScorePrefs;
}

TowerStackerGame::TowerStackerGame(uint32_t width, uint32_t height, uint32_t left)
    : Game("Tower Stacker", width, height), left_(left) {}

bool TowerStackerGame::hasCustomOverlay() const {
  return true;
}

void TowerStackerGame::onGameReset() {
  loadHighScore();
  const int baseW = 36;
  layerCount_ = 1;
  score_ = 0;
  layers_[0].x = (static_cast<int>(width) - baseW) / 2;
  layers_[0].w = baseW;
  movingDir_ = 1;
  movingSpeed_ = 16.0f;
  prepareNextBlock(baseW);
}

void TowerStackerGame::updateRunning(uint32_t deltaMs, const ButtonInput& input) {
  const float deltaSec = static_cast<float>(deltaMs) * 0.001f;
  movingX_ += static_cast<float>(movingDir_) * movingSpeed_ * deltaSec;

  const float minX = 1.0f;
  const float maxX = static_cast<float>(width - movingW_ - 1);
  if (movingX_ <= minX) {
    movingX_ = minX;
    movingDir_ = 1;
  } else if (movingX_ >= maxX) {
    movingX_ = maxX;
    movingDir_ = -1;
  }

  if (input.click) {
    dropMovingBlock();
  }
}

void TowerStackerGame::drawRunning(U8G2& u8g2) {
  u8g2.drawFrame(left_, 0, width, height);
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(left_ + 2, 6);
  u8g2.print(score_);
  u8g2.print("/");
  u8g2.print(highScore_);

  const uint8_t visibleLayers = (height - HUD_H - 2) / BLOCK_H;
  uint8_t firstVisibleLayer = 0;
  if (layerCount_ >= visibleLayers) {
    firstVisibleLayer = layerCount_ - visibleLayers + 1;
  }

  for (uint8_t i = firstVisibleLayer; i < layerCount_; i++) {
    u8g2.drawBox(left_ + layers_[i].x, layerY(i, firstVisibleLayer), layers_[i].w, BLOCK_H);
  }

  if (layerCount_ < MAX_LAYERS) {
    u8g2.drawFrame(left_ + static_cast<int>(movingX_), layerY(layerCount_, firstVisibleLayer), movingW_, BLOCK_H);
  }
}

void TowerStackerGame::drawStart(U8G2& u8g2) {
  u8g2.drawFrame(0, 0, width + 2, height);
  if (highScore_ > 0 && PlayerProfile::showScorePage()) {
    char initials[4];
    PlayerProfile::unpackDottedInitials(highScoreInitials_, initials);
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(3, 10, "Top Score");
    u8g2.setFont(u8g2_font_4x6_tr);
    u8g2.setCursor(3, 24);
    u8g2.print(initials);
    u8g2.print(" ");
    u8g2.print(highScore_);
    u8g2.drawStr(3, 38, "Tap start");
  } else {
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(3, 10, gameTitle());
    u8g2.drawStr(3, 24, "Tap start");
  }
}

void TowerStackerGame::drawEnd(U8G2& u8g2) {
  u8g2.drawFrame(0, 0, width + 2, height);
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(3, 9, "Game Over");
  u8g2.setCursor(3, 19);
  u8g2.print("Score:");
  u8g2.print(score_);
  u8g2.setCursor(3, 29);
  u8g2.print("Best:");
  u8g2.print(highScore_);
  if (highScore_ > 0) {
    char initials[4];
    PlayerProfile::unpackDottedInitials(highScoreInitials_, initials);
    u8g2.print(" ");
    u8g2.print(initials);
  }
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(3, 38, "Tap retry Hold menu");
}

void TowerStackerGame::loadHighScore() {
  if (highScoreLoaded_) {
    return;
  }
  towerScorePrefs.begin("tower", true);
  highScore_ = towerScorePrefs.getUChar("best", 0);
  highScoreInitials_ = towerScorePrefs.getUShort("init", PlayerProfile::defaultInitials());
  towerScorePrefs.end();
  highScoreLoaded_ = true;
}

void TowerStackerGame::saveHighScore() {
  highScoreInitials_ = PlayerProfile::loadInitials();
  towerScorePrefs.begin("tower", false);
  towerScorePrefs.putUChar("best", highScore_);
  towerScorePrefs.putUShort("init", highScoreInitials_);
  towerScorePrefs.end();
}

void TowerStackerGame::dropMovingBlock() {
  const Layer& previous = layers_[layerCount_ - 1];
  const int droppedX = static_cast<int>(movingX_ + 0.5f);
  const int leftEdge = max(droppedX, previous.x);
  const int rightEdge = min(droppedX + movingW_, previous.x + previous.w);
  const int overlapW = rightEdge - leftEdge;

  if (overlapW <= 0) {
    endGame();
    return;
  }

  if (layerCount_ >= MAX_LAYERS) {
    endGame();
    return;
  }

  layers_[layerCount_].x = leftEdge;
  layers_[layerCount_].w = overlapW;
  layerCount_++;
  score_ = layerCount_ - 1;
  if (score_ > highScore_) {
    highScore_ = score_;
    saveHighScore();
  }

  prepareNextBlock(overlapW);
}

void TowerStackerGame::prepareNextBlock(int blockWidth) {
  movingW_ = blockWidth;
  movingX_ = 1.0f;
  movingDir_ = 1;
  movingSpeed_ = min(32.0f, 16.0f + static_cast<float>(score_) * 0.9f);
}

int TowerStackerGame::layerY(uint8_t layer, uint8_t firstVisibleLayer) const {
  const uint8_t visibleIndex = layer - firstVisibleLayer;
  return static_cast<int>(height) - 2 - ((visibleIndex + 1) * BLOCK_H);
}
