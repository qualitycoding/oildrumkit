# Knowledge: listening-tooling facts for oildrumkit

- Percussive 1.0 s onset clips from the kit have a crest factor of 22 to 34 dB; the vendored builder only fits all 15 instruments at about -36 dB RMS.
- The builder's `cut` and `prepare` functions are importable, so a planner can reuse the exact cutting and resampling.
- Seeds: 20261103 for round 1, plus 1000 per later round, three rounds at most.
- The engine clamps pitch to each instrument's range; clamped stems must be named in the round record.
