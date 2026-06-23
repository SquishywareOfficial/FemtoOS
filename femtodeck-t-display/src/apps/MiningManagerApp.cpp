#include "MiningManagerApp.h"

#include <TFT_eSPI.h>

#include "../../TDisplayUi.h"
#include "../shared/logic/MinerClusterLogic.h"

namespace {
constexpr const char* HOSTNAME = "FemtoDeck-Cluster";

template <typename Drawer>
void drawBuffered(TFT_eSPI& tft, uint32_t width, uint32_t height, Drawer drawer) {
  static TFT_eSprite frame(&tft);
  static bool frameReady = false;
  if (!frameReady) {
    frame.setColorDepth(8);
    frameReady = frame.createSprite(width, height) != nullptr;
  }

  if (frameReady) {
    frame.fillSprite(TFT_BLACK);
    drawer(frame);
    frame.pushSprite(0, 0);
  } else {
    tft.fillScreen(TFT_BLACK);
    drawer(tft);
  }
}

template <typename Canvas>
void drawFit(Canvas& canvas, const String& text, int x, int y, int maxWidth, uint8_t size, uint16_t color) {
  canvas.setTextDatum(TL_DATUM);
  canvas.setTextSize(size);
  canvas.setTextColor(color, TFT_BLACK);
  canvas.drawString(TDisplayUi::fitText(canvas, text, maxWidth), x, y);
}

String rateLabel(uint32_t hps) {
  char text[20];
  if (hps >= 1000000UL) {
    snprintf(text, sizeof(text), "%.2f MH/s", hps / 1000000.0f);
  } else if (hps >= 1000UL) {
    snprintf(text, sizeof(text), "%.1f KH/s", hps / 1000.0f);
  } else {
    snprintf(text, sizeof(text), "%lu H/s", static_cast<unsigned long>(hps));
  }
  return String(text);
}
}

MiningManagerApp::MiningManagerApp(uint32_t width, uint32_t height)
    : App("Distributed Miner", width, height), cluster_(new MinerCluster::MasterEngine()) {}

MiningManagerApp::~MiningManagerApp() {
  delete cluster_;
}

bool MiningManagerApp::hasCustomOverlay() const {
  return true;
}

bool MiningManagerApp::startsRunningImmediately() const {
  return true;
}

uint16_t MiningManagerApp::runningRenderIntervalMs() const {
  return 500;
}

bool MiningManagerApp::wantsImmediateRender() const {
  return dirty_;
}

void MiningManagerApp::debugStartCluster() {
  if (!cluster_->running()) {
    cluster_->start(HOSTNAME);
    forceClear();
  }
  debugPrintStats("debug-start");
}

void MiningManagerApp::debugStopCluster() {
  if (cluster_->running()) {
    cluster_->stop();
    forceClear();
  }
  debugPrintStats("debug-stop");
}

void MiningManagerApp::debugStartPairing() {
  if (!cluster_->running()) {
    cluster_->start(HOSTNAME);
  }
  cluster_->startPairing();
  forceClear();
  debugPrintStats("debug-pair");
}

void MiningManagerApp::debugSetLocalMining(bool enabled) {
  cluster_->setLocalMining(enabled);
  forceClear();
  debugPrintStats(enabled ? "local-on" : "local-off");
}

void MiningManagerApp::debugResetCluster() {
  cluster_->resetClusterIdentity();
  forceClear();
  debugPrintStats("cluster-reset");
}

void MiningManagerApp::debugPrintStats(const char* reason) {
  printStatsLine(cluster_->stats(), reason);
}

void MiningManagerApp::onAppReset() {
  page_ = Page::Dashboard;
  lastSerialStatsMs_ = 0;
  forceClear();
}

void MiningManagerApp::onAppExit() {
  cluster_->stop();
}

void MiningManagerApp::markDirty() {
  dirty_ = true;
}

void MiningManagerApp::forceClear() {
  dirty_ = true;
  forceScreenClear_ = true;
}

void MiningManagerApp::updateRunning(uint32_t deltaMs, const ButtonInput& b1, const ButtonInput& b2) {
  (void)deltaMs;
  const uint32_t now = millis();
  if (cluster_->running() && (lastSerialStatsMs_ == 0 || now - lastSerialStatsMs_ >= 2000)) {
    lastSerialStatsMs_ = now;
    printStatsLine(cluster_->stats(), "tick");
  }

  if (b2.click) {
    page_ = static_cast<Page>((static_cast<uint8_t>(page_) + 1) % static_cast<uint8_t>(Page::Count));
    forceClear();
    return;
  }

  if (b1.click) {
    if (page_ == Page::Dashboard) {
      if (cluster_->running()) cluster_->stop();
      else cluster_->start(HOSTNAME);
      forceClear();
    } else if (page_ == Page::Controls) {
      cluster_->setLocalMining(!cluster_->localMiningEnabled());
      forceClear();
    } else if (page_ == Page::Pairing) {
      if (!cluster_->running()) cluster_->start(HOSTNAME);
      cluster_->startPairing();
      forceClear();
    } else if (page_ == Page::Reset) {
      cluster_->resetClusterIdentity();
      forceClear();
    }
    return;
  }

  if (b1.longPress) {
    if (page_ == Page::Reset) {
      cluster_->resetClusterIdentity();
    } else if (page_ == Page::Controls) {
      if (cluster_->running()) cluster_->stop();
      else cluster_->start(HOSTNAME);
    } else {
      if (!cluster_->running()) cluster_->start(HOSTNAME);
      cluster_->startPairing();
    }
    forceClear();
  }
}

void MiningManagerApp::printStatsLine(const MinerCluster::MasterStats& stats, const char* reason) {
  Serial.print("[cluster] reason=");
  Serial.print(reason);
  Serial.print(" state=");
  Serial.print(MinerCluster::masterStateLabel(stats.state));
  Serial.print(" total_khps=");
  Serial.print(stats.totalHps / 1000.0f, 1);
  Serial.print(" local_khps=");
  Serial.print(stats.localHps / 1000.0f, 1);
  Serial.print(" slave_khps=");
  Serial.print(stats.slaveHps / 1000.0f, 1);
  Serial.print(" slaves=");
  Serial.print(stats.slaveCount);
  Serial.print(" local=");
  Serial.print(stats.localMining ? "on" : "off");
  Serial.print(" jobs=");
  Serial.print(stats.jobs);
  Serial.print(" submitted=");
  Serial.print(stats.submitted);
  Serial.print(" accepted=");
  Serial.print(stats.accepted);
  Serial.print(" rejected=");
  Serial.print(stats.rejected);
  Serial.print(" channel=");
  Serial.print(stats.channel);
  if (stats.lastError[0]) {
    Serial.print(" error=");
    Serial.print(stats.lastError);
  }
  Serial.println();
}

const char* MiningManagerApp::pageTitle() const {
  switch (page_) {
    case Page::Slaves: return "Cluster Slaves";
    case Page::Pool: return "Cluster Pool";
    case Page::Pairing: return "Pair Slaves";
    case Page::Controls: return "Cluster Control";
    case Page::Reset: return "Reset Cluster";
    case Page::Dashboard:
    default: return "Distributed Miner";
  }
}

uint16_t MiningManagerApp::stateColor(MinerCluster::MasterState state) const {
  switch (state) {
    case MinerCluster::MasterState::Mining: return TFT_GREEN;
    case MinerCluster::MasterState::Idle: return TFT_CYAN;
    case MinerCluster::MasterState::Stopping: return TFT_ORANGE;
    case MinerCluster::MasterState::NoWifi:
    case MinerCluster::MasterState::WifiFailed:
    case MinerCluster::MasterState::RadioFailed:
    case MinerCluster::MasterState::PoolFailed:
    case MinerCluster::MasterState::Error: return TFT_RED;
    default: return TFT_YELLOW;
  }
}

template <typename Canvas>
void MiningManagerApp::drawDashboard(Canvas& canvas, const MinerCluster::MasterStats& stats) {
  const uint16_t color = stateColor(stats.state);
  TDisplayUi::header(canvas, pageTitle(), color, MinerCluster::masterStateLabel(stats.state));
  TDisplayUi::largeValue(canvas, rateLabel(stats.totalHps), 39, color);
  canvas.setTextSize(1);
  canvas.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  canvas.drawString(String("Local ") + rateLabel(stats.localHps) + (stats.localMining ? " on" : " off"), 16, 84);
  canvas.drawString(String("Slaves ") + stats.slaveCount + "  " + rateLabel(stats.slaveHps), 16, 99);
  TDisplayUi::footer(canvas, cluster_->running() ? "B1 stop  B2 page  B1 hold pair" : "B1 start  B2 page");
}

template <typename Canvas>
void MiningManagerApp::drawSlaves(Canvas& canvas) {
  TDisplayUi::header(canvas, pageTitle(), TFT_CYAN);
  MinerCluster::SlaveRecord slave;
  bool any = false;
  for (uint8_t row = 0; row < 4; row++) {
    if (!cluster_->slaveAt(row, slave)) break;
    any = true;
    const int y = 34 + row * 21;
    canvas.setTextSize(1);
    canvas.setTextColor(TFT_CYAN, TFT_BLACK);
    canvas.drawString(MinerCluster::macSuffix(slave.mac), 12, y);
    canvas.setTextColor(TFT_WHITE, TFT_BLACK);
    canvas.drawString(rateLabel(slave.hashrate), 73, y);
    drawFit(canvas, slave.status, 158, y, 70, 1, TFT_LIGHTGREY);
  }
  if (!any) {
    TDisplayUi::centered(canvas, "No slaves yet", 52, 2, TFT_LIGHTGREY);
    TDisplayUi::centered(canvas, "Use pair page", 78, 1, TFT_CYAN);
  }
  TDisplayUi::footer(canvas, "B2 page");
}

template <typename Canvas>
void MiningManagerApp::drawPool(Canvas& canvas, const MinerCluster::MasterStats& stats) {
  const MinerCluster::MinerConfig config = MinerCluster::loadMinerConfig();
  TDisplayUi::header(canvas, pageTitle(), stateColor(stats.state));
  drawFit(canvas, String("WiFi: ") + (stats.ssid[0] ? stats.ssid : "-"), 14, 34, 210, 1, TFT_WHITE);
  drawFit(canvas, String("Pool: ") + config.poolHost + ":" + config.poolPort, 14, 50, 210, 1, TFT_CYAN);
  drawFit(canvas, String("Jobs: ") + stats.jobs + "  Diff: " + String(stats.poolDifficulty, 5), 14, 66, 210, 1, TFT_LIGHTGREY);
  drawFit(canvas, String("OK/Rej: ") + stats.accepted + "/" + stats.rejected, 14, 82, 210, 1, TFT_GREEN);
  drawFit(canvas, String("Worker: ") + MinerCluster::workerName(), 14, 98, 210, 1, TFT_YELLOW);
  TDisplayUi::footer(canvas, "B2 page");
}

template <typename Canvas>
void MiningManagerApp::drawPairing(Canvas& canvas, const MinerCluster::MasterStats& stats) {
  const bool radioReady = stats.channel != 0;
  const uint16_t color = stats.pairing && radioReady ? TFT_GREEN : (stats.pairing ? TFT_YELLOW : TFT_ORANGE);
  TDisplayUi::header(canvas, pageTitle(), color, MinerCluster::masterStateLabel(stats.state));
  if (stats.pairing && radioReady) {
    TDisplayUi::centered(canvas, "Pairing Open", 39, 2, TFT_GREEN);
    TDisplayUi::centered(canvas, String("Channel ") + stats.channel, 66, 2, TFT_CYAN);
  } else if (stats.pairing) {
    TDisplayUi::centered(canvas, "Radio Waiting", 39, 2, TFT_YELLOW);
    TDisplayUi::centered(canvas, "Need WiFi first", 67, 1, TFT_LIGHTGREY);
  } else {
    TDisplayUi::centered(canvas, "Pairing Closed", 43, 2, TFT_ORANGE);
    TDisplayUi::centered(canvas, "B1 opens 60 sec", 70, 1, TFT_LIGHTGREY);
  }
  TDisplayUi::centered(canvas, String(stats.slaveCount) + " slaves", 96, 1, TFT_LIGHTGREY);
  TDisplayUi::footer(canvas, "B1 pair  B2 page");
}

template <typename Canvas>
void MiningManagerApp::drawControls(Canvas& canvas, const MinerCluster::MasterStats& stats) {
  TDisplayUi::header(canvas, pageTitle(), TFT_YELLOW);
  TDisplayUi::row(canvas, 38, stats.localMining ? "Local Mining: On" : "Local Mining: Off", true, TFT_YELLOW);
  TDisplayUi::row(canvas, 65, cluster_->running() ? "Cluster: Running" : "Cluster: Stopped", false, TFT_CYAN);
  TDisplayUi::centered(canvas, "B1 toggle local", 96, 1, TFT_LIGHTGREY);
  TDisplayUi::footer(canvas, "B1 local  B1 hold start/stop");
}

template <typename Canvas>
void MiningManagerApp::drawReset(Canvas& canvas, const MinerCluster::MasterStats& stats) {
  TDisplayUi::header(canvas, pageTitle(), TFT_RED, MinerCluster::masterStateLabel(stats.state));
  TDisplayUi::centered(canvas, "Forget slaves", 38, 2, TFT_ORANGE);
  TDisplayUi::centered(canvas, "New cluster id", 64, 2, TFT_CYAN);
  TDisplayUi::centered(canvas, stats.localMining ? "Local mining kept ON" : "Local mining kept OFF", 95, 1, TFT_LIGHTGREY);
  TDisplayUi::footer(canvas, "B1 reset  B2 page");
}

template <typename Canvas>
void MiningManagerApp::drawFrame(Canvas& canvas) {
  TDisplayUi::clear(canvas);
  const MinerCluster::MasterStats stats = cluster_->stats();
  switch (page_) {
    case Page::Slaves:
      drawSlaves(canvas);
      break;
    case Page::Pool:
      drawPool(canvas, stats);
      break;
    case Page::Pairing:
      drawPairing(canvas, stats);
      break;
    case Page::Controls:
      drawControls(canvas, stats);
      break;
    case Page::Reset:
      drawReset(canvas, stats);
      break;
    case Page::Dashboard:
    default:
      drawDashboard(canvas, stats);
      break;
  }
  dirty_ = false;
}

void MiningManagerApp::drawRunning(TFT_eSPI& tft) {
  tft.setRotation(1);
  if (forceScreenClear_) {
    tft.fillScreen(TFT_BLACK);
    forceScreenClear_ = false;
  }
  drawBuffered(tft, width, height, [this](auto& canvas) { drawFrame(canvas); });
}

void MiningManagerApp::drawStart(TFT_eSPI& tft) {
  tft.setRotation(1);
  TDisplayUi::clear(tft);
  if (showStartPromptPage()) {
    TDisplayUi::header(tft, "Distributed Miner", TFT_ORANGE);
    TDisplayUi::centered(tft, "Press", 50, 3, TFT_WHITE);
    TDisplayUi::centered(tft, "to Start", 80, 2, TFT_LIGHTGREY);
    TDisplayUi::footer(tft, "B1 start");
  } else if (showStartScorePage()) {
    TDisplayUi::header(tft, "Cluster Mining", TFT_YELLOW);
    TDisplayUi::centered(tft, "T-Display master", 49, 2, TFT_CYAN);
    TDisplayUi::centered(tft, "C3 headless slaves", 78, 1, TFT_LIGHTGREY);
    TDisplayUi::footer(tft, "B1 start");
  } else {
    TDisplayUi::header(tft, "Distributed Miner", TFT_ORANGE);
    tft.drawCircle(90, 66, 20, TFT_ORANGE);
    tft.drawString("M", 84, 57, 4);
    for (int i = 0; i < 3; i++) {
      const int x = 140 + i * 22;
      tft.drawRect(x, 52, 16, 22, TFT_CYAN);
      tft.drawFastHLine(x + 3, 69, 10, TFT_GREEN);
    }
    TDisplayUi::footer(tft, "B1 start");
  }
}
