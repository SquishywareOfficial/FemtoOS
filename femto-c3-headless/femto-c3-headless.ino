#include <Arduino.h>

#include "Version.h"
#include "src/shared/logic/MinerClusterLogic.h"

constexpr uint8_t LED_PIN = 8;
constexpr uint8_t BUTTON_PIN = 9;

// Headless C3 miner controls:
// - Boots directly as a Distributed Miner slave.
// - Hold BOOT for 5 seconds to forget the saved master/cluster pairing.
// - GPIO8 LED is active-low on the ESP32-C3 SuperMini boards tested here.
// - LED states:
//   - solid: actively or recently mining
//   - slow blink: searching / not paired
//   - very slow blink: paired but idle
//   - fast blink: error
constexpr bool LED_ACTIVE_LOW = true;
constexpr uint32_t CLEAR_HOLD_MS = 5000;

MinerCluster::SlaveEngine slave;

uint32_t buttonPressedAtMs = 0;
bool clearEmitted = false;
uint32_t lastStatsAtMs = 0;
uint32_t lastLedAtMs = 0;
uint32_t lastMiningSeenAtMs = 0;
bool ledOn = false;

bool buttonDown() {
  return digitalRead(BUTTON_PIN) == LOW;
}

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, LED_ACTIVE_LOW ? (on ? LOW : HIGH) : (on ? HIGH : LOW));
}

void updateButton(uint32_t nowMs) {
  if (buttonDown()) {
    if (buttonPressedAtMs == 0) {
      buttonPressedAtMs = nowMs;
      clearEmitted = false;
    } else if (!clearEmitted && nowMs - buttonPressedAtMs >= CLEAR_HOLD_MS) {
      clearEmitted = true;
      slave.clearPairing();
      Serial.println("[headless] pairing cleared");
      for (uint8_t i = 0; i < 6; i++) {
        setLed((i & 1) == 0);
        delay(120);
      }
    }
  } else {
    buttonPressedAtMs = 0;
    clearEmitted = false;
  }
}

void updateLed(uint32_t nowMs) {
  const MinerCluster::SlaveStats stats = slave.stats();
  uint16_t interval = 1000;
  if (stats.state == MinerCluster::SlaveState::Mining) {
    lastMiningSeenAtMs = nowMs;
  }
  if (nowMs - lastMiningSeenAtMs < 3000) {
    setLed(true);
    return;
  }
  if (stats.state == MinerCluster::SlaveState::Error) {
    interval = 120;
  } else if (!stats.paired || stats.state == MinerCluster::SlaveState::Searching) {
    interval = 700;
  } else {
    interval = 2000;
  }

  if (nowMs - lastLedAtMs >= interval) {
    lastLedAtMs = nowMs;
    setLed(!ledOn);
  }
}

void printStats(uint32_t nowMs) {
  if (nowMs - lastStatsAtMs < 2000) return;
  lastStatsAtMs = nowMs;
  const MinerCluster::SlaveStats stats = slave.stats();
  Serial.print("[headless] build=");
  Serial.print(BuildInfo::BUILD_TEXT);
  Serial.print(" state=");
  Serial.print(MinerCluster::slaveStateLabel(stats.state));
  Serial.print(" paired=");
  Serial.print(stats.paired ? "yes" : "no");
  Serial.print(" channel=");
  Serial.print(stats.channel);
  Serial.print(" khps=");
  Serial.print(stats.hashrate / 1000.0f, 1);
  Serial.print(" jobs=");
  Serial.print(stats.jobs);
  Serial.print(" completed=");
  Serial.print(stats.completed);
  Serial.print(" total_kh=");
  Serial.print(static_cast<unsigned long>(stats.totalHashes / 1000ULL));
  if (stats.lastError[0]) {
    Serial.print(" error=");
    Serial.print(stats.lastError);
  }
  Serial.println();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  setLed(false);
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.print("[headless] Femto C3 Headless ");
  Serial.println(BuildInfo::BUILD_TEXT);
  Serial.println("[headless] role=Miner Slave");
  Serial.println("[headless] hold BOOT for 5s to clear pairing");
  slave.begin();
}

void loop() {
  const uint32_t now = millis();
  updateButton(now);
  slave.update();
  updateLed(now);
  printStats(now);
  delay(1);
}
