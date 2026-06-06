# Release Notes

Version `1.0` is the original atomic14 baseline. Build numbers are tracked from the local expanded build history onward.

To create a new build with an incremented build number, run:

```powershell
.\build.ps1
```

If compiling manually with `arduino-cli compile`, bump `esp32-c3-games/Version.h` first and add a note here.

## v1.1 b12

- Fixed Options initials save and Credits long-hold behavior so both return directly to the main menu.
- Added clearer `Tap start`/`Tap retry` prompts to splash, score, and utility screens.
- Made Need Speed level 1 more reachable, added explicit `Lost` result text, and shows the target speed/time when you miss.
- Added Fishing Flick's hook-the-fish timing phase and softened tension recovery so fight spikes are less punishing.
- Moved Blackjack hand totals left to avoid the OLED border and added clearer start prompts.
- Widened Tiny Golf's tight early wall gap.
- Added a desktop level editor at `sim/editor.html` for Tiny Golf and maze wall layouts.
- Synced the browser simulator with the gameplay/prompt changes.

## v1.1 b11

- Added `build.sh`, a Bash build helper for Linux and macOS that increments `Version.h` before running `arduino-cli compile`.
- Bumped the firmware build for device upload.

## v1.1 b10

- Added `Version.h` with `BuildInfo::VERSION_TEXT` and `BuildInfo::BUILD_TEXT`.
- Displayed the current build number on the Credits start and end screens.
- Added `build.ps1` to increment the build number before running `arduino-cli compile`.
- Added this release notes file.

## v1.1 b9

- Mini Lander now shows the safe landing velocity on the crash/end screen as a speed-limit style sign.
- Mini Lander safe landing velocity now varies by level.
- Mini Lander level briefing now ignores input for 1 second, then waits for a fresh tap before starting.
- Need Speed now has 5 levels, increasing target speed and rev difficulty between levels.
- Need Speed now shows a level-clear interstitial and total run time.
- Fishing Flick now has level progression with larger, harder fish.
- Fishing Flick tension now spikes during fish fights, can break at high tension, and can lose the fish at zero slack.

## v1.1 b8

- Added an `Options` menu entry for two-character player initials.
- Added shared `PlayerProfile` storage for initials.
- High-score games now save initials alongside new best scores.
- Intro screens for score-saving games now alternate between splash art and top-score display.
- Added `Maze Collector`, a key-collection variant of Maze Runner with a locked exit.
- Improved Maze Runner defensiveness around no-option choice states.
- Reworked Pipe Mania to make the selected square clearer, slow early levels, and invert filled liquid cells.
- Reworked Blackjack with bet selection, animated dealing, a persistent two-deck shoe, reshuffle behavior, and delayed dealer actions.
- Updated Credits with per-game pages for new games and helpers.

## v1.1 b7

- Added browser emulator under `sim/`.
- Added a crisp bitmap font to the emulator so the scaled OLED text remains readable.
- Emulator supports the one-button menu, GPIO8 LED indicator, local score storage, and playable versions of the new games.

## v1.1 b6

- Added Noon Shooter.
- Added Fishing Flick with GPIO8 tension warning.
- Added Maze Runner with generated mazes and one-button intersection choices.
- Added Pipe Mania adapted for one-button pipe placement.
- Added Blackjack.

## v1.1 b5

- Added Need Speed, a one-button drag-racing dashboard game.
- Added traffic-light launch countdown.
- Added round tachometer, shift quality messages, and GPIO8 shift-light behavior.

## v1.1 b4

- Added Credits as a menu entry.
- Added per-game credit pages for the original atomic14 games and new local games.
- Added GPIO8 support patterns for games that use the onboard LED.

## v1.1 b3

- Added Mini Lander.
- Added moon splash screen, level briefings, fuel/altitude/velocity HUD, landing pad, thrust physics, crash/landing outcomes, and GPIO8 burn/low-fuel cues.

## v1.1 b2

- Added Tiny Golf.
- Added rotating aim, power selection, walls, cups, bounce guides, multi-hole course flow, and best-score persistence.
- Tuned early golf courses, bumper placement, cup forgiveness, aim speed near the cup, and shot behavior.

## v1.1 b1

- Added Tower Stacker.
- Added score display and persistent high score.
- Added end-screen score display.
- Added shared end-screen input lockout to reduce accidental restarts across games.

## v1.0 b0

- Original atomic14 project baseline.
- Included Breakout, Micro Racer, Defender Mini, Jump Run, and Heli Cave.
- Included the original one-button menu flow and ESP32-C3 OLED display setup.
