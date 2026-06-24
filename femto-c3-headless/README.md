# Femto C3 Headless

Headless ESP32-C3 miner slave firmware for FemtoDeck Distributed Miner.

This sketch is for ESP32-C3 SuperMini style boards without a display. It boots straight into Distributed Miner slave mode and waits for a FemtoDeck master, usually the T-Display `Distributed Miner` app, to pair and assign mining work.

## Flash

Compile:

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app femto-c3-headless
```

Upload, replacing `COM11` with the connected port:

```powershell
arduino-cli upload --fqbn esp32:esp32:esp32c3:PartitionScheme=huge_app --port COM11 femto-c3-headless
```

## Pairing

1. Flash and power the headless C3.
2. On the T-Display, open `Distributed Miner`.
3. Start the cluster and open pairing mode.
4. The headless C3 should pair automatically and begin mining once the master has pool work to distribute.

If the C3 was paired to another master or cluster, hold the BOOT button for 5 seconds to clear the saved pairing. The LED flashes rapidly a few times after the pairing is cleared.

## LED States

The tested ESP32-C3 SuperMini board uses GPIO8 for the blue LED, and the LED is active-low.

| LED behavior | Meaning |
| --- | --- |
| Solid on | Actively or recently mining |
| Slow blink | Searching / not paired |
| Very slow blink | Paired but idle |
| Fast blink | Error |

The LED stays solid for a short grace period after a mining assignment finishes. Distributed mining assignments are small, so the slave can move quickly between `Mining`, `Paired`, and `Assigned` states even while the master is receiving useful work.

## Button

| Action | Result |
| --- | --- |
| Hold BOOT for 5 seconds | Clear saved master/cluster pairing |

## Serial Output

Serial runs at `115200` baud and prints a status line every 2 seconds:

```text
[headless] build=<build> state=<state> paired=<yes/no> channel=<n> khps=<rate> jobs=<jobs> completed=<count> total_kh=<hashes>
```

This is useful when checking whether the slave is paired, which WiFi channel it is using, and whether assignments are being completed.
