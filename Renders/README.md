# A/B renders

Same MIDI pattern, same 48 kHz, same Wood Stick, default pitches; only the engine differs.

| file | what |
|---|---|
| `before/oildrumkit_before.wav` (+ `_A`..`_D`) | legacy engine (mono), 16-bit, **peak-normalised to -1 dBFS** |
| `after/oildrumkit_after.wav` (+ `_A`..`_D`) | modal engine (stereo, room 12 %), 16-bit, peak-normalised to -1 dBFS |
| `*/oildrumkit_*.meta.json` | raw peak and the normalisation gain that was applied |
| `*/spectra_XX_*.png` | v=1.0 single hit, dry: spectrum / spectrogram / per-octave RT60 |
| `compare/compare_XX_*.png`, `compare/comparison.md` | before/after overlays and the metric table with pass/fail |

Sections: **A** each instrument once (3.0 s apart) - **B** each instrument at velocities 0.2/0.45/0.7/1.0 -
**C** 8-bar groove at 96 BPM - **D** snare roll (32nds).

**Loudness caveat.** Both files are peak-normalised, which is *not* loudness-matched. The legacy engine's raw output peaks at
**+11.9 dBFS** (it clips: single full-velocity bass/snare/tom hits alone peak at +4 to +6 dBFS), so normalising it to
-1 dBFS makes it much quieter than it plays in a DAW; the new engine's raw peak is -5.3 dBFS. Match levels by ear.

**The dry metrics were computed on dry renders** (`render_demo metrics`, `veltest`, and a room=0 demo), not on these
room-mix files.

## What to listen for
* **Velocity** (section B): the legacy engine only changes level; the new one adds a mild brightening (see comparison.md).
* **Rolls** (section D): repeated snare hits should no longer be near-identical in the 100-1000 Hz band.
* **Cymbals/hats**: dense shimmer over a fast-decaying top, instead of filtered hiss.
* If something is wrong, `Tools/voicing.json` is where to fix it (damping `dampScale`, `nl`, `residual`, `shellAmp`, pans ...).

## Not verified
No listening test or real-recording comparison was possible during development. The cowbell now has *fewer*
resolved partials than before (2 vs 4): the old recipe hand-set the classic 1 : 1.48 pair, the new one uses plate modes
and probably needs retuning by ear.
