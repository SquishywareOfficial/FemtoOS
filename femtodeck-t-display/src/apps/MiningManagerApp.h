#pragma once

#include "../../App.h"

class TFT_eSPI;

namespace MinerCluster {
class MasterEngine;
struct MasterStats;
enum class MasterState : uint8_t;
}

class MiningManagerApp : public App {
public:
  MiningManagerApp(uint32_t width, uint32_t height);
  ~MiningManagerApp() override;

  bool hasCustomOverlay() const override;
  uint16_t runningRenderIntervalMs() const override;
  bool wantsImmediateRender() const override;

  void debugStartCluster();
  void debugStopCluster();
  void debugStartPairing();
  void debugSetLocalMining(bool enabled);
  void debugResetCluster();
  void debugPrintStats(const char* reason = "debug");

protected:
  bool startsRunningImmediately() const override;
  void onAppReset() override;
  void onAppExit() override;
  void updateRunning(uint32_t deltaMs, const ButtonInput& b1, const ButtonInput& b2) override;
  void drawRunning(TFT_eSPI& tft) override;
  void drawStart(TFT_eSPI& tft) override;

private:
  enum class Page : uint8_t {
    Dashboard,
    Slaves,
    Pool,
    Pairing,
    Controls,
    Reset,
    Count
  };

  void markDirty();
  void forceClear();
  void printStatsLine(const MinerCluster::MasterStats& stats, const char* reason);
  const char* pageTitle() const;
  uint16_t stateColor(MinerCluster::MasterState state) const;

  template <typename Canvas>
  void drawFrame(Canvas& canvas);
  template <typename Canvas>
  void drawDashboard(Canvas& canvas, const MinerCluster::MasterStats& stats);
  template <typename Canvas>
  void drawSlaves(Canvas& canvas);
  template <typename Canvas>
  void drawPool(Canvas& canvas, const MinerCluster::MasterStats& stats);
  template <typename Canvas>
  void drawPairing(Canvas& canvas, const MinerCluster::MasterStats& stats);
  template <typename Canvas>
  void drawControls(Canvas& canvas, const MinerCluster::MasterStats& stats);
  template <typename Canvas>
  void drawReset(Canvas& canvas, const MinerCluster::MasterStats& stats);

  MinerCluster::MasterEngine* cluster_;
  Page page_ = Page::Dashboard;
  bool dirty_ = true;
  bool forceScreenClear_ = true;
  uint32_t lastSerialStatsMs_ = 0;
};
