> **Archived earlier engine.** This folder is the first (additive sine + noise) Oil Drum Kit design with its seven retrospective defects fixed. It is NOT the engine on `main`, which is the later physical-modelling rewrite. Kept for reference and for its tests; paths below are relative to this folder.

# Oil Drum Kit — VST3 / Standalone instrument (JUCE source)

Fifteen synthesised percussion voices, each with its own tunable pitch: Bass Drum, Snare 1 and 2,
Low / Mid / High Tom (two of each), Splash, Ride, Hi-Hat Closed, Hi-Hat Open, metal Cowbell and
Rimshot. No samples. Drums, cowbell and rimshot are sine and resonant-noise partials at inharmonic
ratios of the tuning frequency; cymbals and hats are a bank of six band-limited square waves plus
noise through a steep high-pass. The result is a design target ("struck sheet steel"), not a
measured copy of real oil drums or real cymbals, so judge it by ear.

## Status — what has and has not been checked

| Item | Status |
|---|---|
| Engine (`Source/DrumEngine.h`) | `tests/test_engine.cpp`: 134 checks, 0 failed (g++ 13, Linux). |
| VST3 build | **Compiled and linked** with JUCE 7.0.12 (CMake 3.28, g++, Ubuntu Linux x86-64), target `OilDrumKit_VST3`. |
| Standalone build | **Not attempted.** |
| macOS / Windows builds | **Not built.** |
| Loading in a DAW / plugin host | **Not tested.** The editor has never been displayed and the plugin never run inside a host. |
| Web preview | `engine.js` matches the C++ engine in 48 of 48 sample-by-sample checks at each of 44.1, 48 and 96 kHz (`tests/test_web_parity.js`). The page loads in headless Chromium and all 15 pads produce sound with no script errors (`tests/browser_smoke.js`). Actual listening in a browser was not done. |
| Sound quality | Not judged by a human listener. |

## Build the plugin

Requires CMake ≥ 3.22, a C++17 compiler and an internet connection (CMake fetches JUCE 7.0.12).
On Linux you also need JUCE's usual development packages (ALSA, X11, FreeType, fontconfig, GL).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target OilDrumKit_VST3     # or omit --target for VST3 + Standalone
```

`COPY_PLUGIN_AFTER_BUILD` is on, so the VST3 is also copied to your user VST3 folder
(Linux `~/.vst3/`, macOS `~/Library/Audio/Plug-Ins/VST3/`, Windows `C:\Program Files\Common Files\VST3\`).
Rescan plugins in your DAW afterwards. Set it to FALSE in `CMakeLists.txt` if you do not want that.

## MIDI map

The one table is `kKit` in `Source/KitMap.h`; the editor and the processor both read it. Where General
MIDI has a matching channel-10 instrument the note follows it (37 is GM "Side Stick", used here for
Rimshot; 40 is GM "Electric Snare", used for Snare 2). This mapping comes from a single source.

| Note | Instrument | Note | Instrument |
|---|---|---|---|
| 36 | Bass Drum | 45 | Mid Tom 1 |
| 37 | Rimshot | 46 | Hi-Hat Open |
| 38 | Snare 1 | 47 | Mid Tom 2 |
| 40 | Snare 2 | 48 | High Tom 1 |
| 41 | Low Tom 1 | 50 | High Tom 2 |
| 42 | Hi-Hat Closed | 51 | Ride |
| 43 | Low Tom 2 | 55 | Splash |
| | | 56 | Cowbell |

Closed and Open hi-hat choke each other. Velocity scales loudness. Note events are applied at the
start of each audio block, not at their exact sample position.

## Tuning

Each instrument has one automatable parameter, its fundamental in Hz (ranges and defaults come from
`kitRecipes()` in `Source/DrumEngine.h`; e.g. Bass Drum 30–120 Hz, default 55). **The pitch is taken
when a drum is struck:** turning a knob changes the next hit, not a note that is already ringing
(checked in `tests/test_engine.cpp`, section 5). Automation therefore changes pitch hit by hit.

## Output level

The sum of all voices goes through a fixed gain (`kMasterGain` = 0.25) and a `tanh` soft clip inside
`DrumEngine::render()`, so the output never reaches ±1. Measured with the tests: Bass + Snare 1 +
Ride + Low Tom 1 together peak at 0.874; all 15 voices at velocity 1 together peak at 0.998
(the soft clip is working hard there). A single hit at velocity 1 peaks between 0.1 (cymbals) and 0.5 (drums). Use your host's channel
fader for more or less level.

## Web preview

Open `web/oildrumkit.html` (one self-contained file) in a browser, tune with the sliders, click a lid or use the
keys shown on the pads. It runs `web/engine.js`, a line-by-line port of the C++ engine, using the same
recipes (`web/kit.json`, generated from the C++ tables). It is a preview of the same algorithm, not the
plugin itself. The page links to Google Fonts for its typeface; it works without them.

## Tests and tools (all optional)

```bash
mkdir -p build
g++ -O2 -std=c++17 -Wall -Wextra -I Source tests/test_engine.cpp -o build/test_engine && build/test_engine

# web parity: regenerate the C++ reference renders, then compare the JavaScript port
g++ -O2 -std=c++17 -I Source tools/dump_reference.cpp -o build/dump_reference
mkdir -p build/ref && build/dump_reference build/ref && node tests/test_web_parity.js build/ref

# after changing any recipe in DrumEngine.h: re-export the kit and rebuild the web page
g++ -std=c++17 -I Source tools/export_recipes.cpp -o build/export_recipes
build/export_recipes > web/kit.json && python3 tools/build_web.py

# audio renders (no normalisation; they exit non-zero if a peak would reach 1.0)
g++ -O2 -std=c++17 -I Source -I tools tools/render_demo.cpp -o build/render_demo && build/render_demo demo.wav
```

`tests/browser_smoke.js` needs Playwright and Chromium (`NODE_PATH=$(npm root -g) node tests/browser_smoke.js web/oildrumkit.html`).

## Changing the sound

Every partial, ratio, decay, noise band and tuning range is in `detail::buildKitRecipes()` in
`Source/DrumEngine.h`. `P(ratio, amp, decay)` is a sine partial and `N(ratio, amp, decay, q)` a
resonant-noise partial. Parameter ranges are read from there, so nothing else needs editing; re-run the
tests afterwards (several check measured levels and cymbal brightness).

## Files in this archive

```
CMakeLists.txt
README.md
Source/   DrumEngine.h  KitMap.h  PluginProcessor.h/.cpp  PluginEditor.h/.cpp
tests/    test_engine.cpp  test_web_parity.js  browser_smoke.js
tools/    export_recipes.cpp  dump_reference.cpp  render_demo.cpp  render_oneshots.cpp  wav.h  build_web.py
web/      oildrumkit.html  oildrumkit.template.html  engine.js  kit.json
audio/    OilDrumKit_demo.wav
```
