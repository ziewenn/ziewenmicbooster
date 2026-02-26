# Mic Booster

A lightweight Windows microphone booster with built-in 3-band EQ, VST3 plugin support, and auto-updates.

## Download

👉 **[Download the latest release](https://github.com/ziewenn/ziewenmicbooster/releases/latest)**

Just download `Mic.Booster.exe` and run it — no installation needed.

## Features

- **Microphone Boost** — Adjustable gain from -20 dB to +40 dB
- **3-Band EQ** — Bass (200 Hz), Mid (1 kHz), Treble (4 kHz) with ±12 dB range
- **VST3 Plugin Support** — Load any VST3 plugin into the audio chain
- **Input/Output Device Selection** — Choose your mic and output device
- **Live Level Meters** — Real-time input and output monitoring
- **Settings Persistence** — All settings saved automatically between sessions
- **System Tray** — Minimizes to tray on close, right-click for menu
- **Launch on Startup** — Optional auto-start with Windows
- **Auto-Updates** — Checks GitHub for new releases and updates in one click

## Usage

1. Run `Mic Booster.exe`
2. Select your input microphone and output device
3. Adjust boost gain and tone (Bass, Mid, Treble)
4. Optionally load a VST3 plugin for additional processing
5. Close the window — it minimizes to the system tray
6. Right-click the tray icon to quit

## Building from Source

### Requirements

- Windows 10/11
- Visual Studio 2022+ (or VS 18 Insiders) with "Desktop development with C++" workload
- CMake 3.15+
- Ninja (optional, recommended)

### Steps

```bash
git clone --recursive https://github.com/ziewenn/ziewenmicbooster.git
cd ziewenmicbooster
```

Open a **x64 Developer Command Prompt** and run:

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Or simply run `build.bat`.

The executable will be at `build/MicBooster_artefacts/Release/Mic Booster.exe`.

## Creating a Release

1. Update the version in `Source/UpdateChecker.h` (`CURRENT_VERSION`)
2. Build the exe
3. Create a new GitHub release with a tag like `v1.1.0`
4. Attach `Mic Booster.exe` as a release asset

The app will automatically notify existing users about the update.

## License

This project uses the [JUCE framework](https://juce.com/) under the AGPLv3 license.
