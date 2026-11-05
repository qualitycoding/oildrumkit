# Spike S1: rebuild and re-render the current code (2026-11-05)

Commands (run from a clone of `qualitycoding/oildrumkit` at `main` 1900ac8; paths shortened):

```
bash Tools/build_render.sh new $SPIKE/render_new          -> "OK -> .../render_new"
cd $SPIKE && ./render_new demo out_after                  -> "rendered 91.5 s, peak 0.544 (-5.3 dBFS), channels=2"
sha256sum out_after.wav                                   -> e487c3fbbba114b5516c54d37ca37efb2e101f0ef717038e70b187286387135d
cmp out_after.wav Renders/after/oildrumkit_after.wav      -> identical
```

Findings:
* `git diff --stat 35fbfea..main -- Source Tools CMakeLists.txt Renders` is empty: the code the listening phase judges is
  the modal engine as first delivered. Only docs and planning files changed on `main` since then.
* Toolchain here: g++ (system), Python 3 with numpy 2.5.3, scipy 1.18.1, soundfile 0.14.0 (soundfile needed `pip install`).
* `render_demo metrics OUT_DIR` writes 60 single-hit clips `iXX_vYY.wav` (15 instruments x velocities 20/45/70/100), dry,
  48 kHz stereo float32, 3 s. These are usable as candidate clips.
* The demo renderer has no per-event hammer, strike, damping, room or pitch control; a new event renderer is needed for
  matched candidates (see D-004).
