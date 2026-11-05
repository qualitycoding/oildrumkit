# Spike S2: the vendored blind-pair builder on this kit's hits (2026-11-05)

Setup: 15 stand-in "references" (the candidate's own `iXX_v100` clips, mono, 16-bit, 44.1 kHz, -12 dB) against 15 candidate
clips (float32, 48 kHz, stereo). Builder: `qualitycoding/skills` `make_blind_pairs.py`, sha256
3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7 (identical to the copy vendored on
`oildrumkit` `impl/listening-validation`).

1. **`--rms-db -23` (the value frozen by D-981) fails on percussion.**
   `clip would exceed full scale at the requested RMS; lower --rms-db`, exit 1. Only 4 of 15 stems build at -23.
   Crest factor (peak over RMS, 1.0 s window from the onset) of the 15 candidate hits is 22.2 to 33.9 dB (rimshot, 33.9).
   Stems that build, by requested level: -23: 4/15; -28: 13/15; -30: 14/15; -33: 14/15; -36: 15/15.
2. **At `--rms-db -36` all 15 stems build** and 12 pairs are written (`key.json.b64`, `sources.json.b64`, 24 WAVs).
3. **Independent decode check (T-L02 algorithm, `s2_verify_refs.py`).** It reads the source with the standard-library
   `wave` module (not soundfile), resamples, cuts from the same -40 dB onset rule and takes the best-lag normalised
   correlation with the reference-side clip of every pair. Good bundle: worst 0.99999 (exit 0). Bundle with one reference-side
   clip replaced by white noise: worst 0.021 (exit 1). This is the check demanded by lesson
   L-20261106T010000Z-verify-blind-references-against-sources.
4. The builder's `cut` and `prepare` functions are importable (`import make_blind_pairs`), so a level planner can reuse the
   exact cutting and resampling the builder uses.
