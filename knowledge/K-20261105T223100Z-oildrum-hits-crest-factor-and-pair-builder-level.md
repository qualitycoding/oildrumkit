---
id: K-20261105T223100Z-oildrum-hits-crest-factor-and-pair-builder-level
statement: "The fifteen oildrumkit instruments' 1.0 s onset clips have a crest factor of 22.2 to 33.9 dB, so make_blind_pairs.py at its default -23 dB RMS builds only 4 of 15 stems and needs about -36 dB for all 15."
status: current
supersedes: []
tags: [subject:oildrumkit-listening-phase, domain:audio, tech:python, phase:planning]
applies_to_version: "oildrumkit 1900ac8; make_blind_pairs.py sha256 3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7"
source: { url: "https://github.com/qualitycoding/skills/blob/108db63/skills/audio-blind-listening-validation/scripts/make_blind_pairs.py", doi: "", title: "Spike S2 (make_blind_pairs.py on 15 dry hits)", locator: "research/spikes/S2_RESULT.md", accessed: "2026-11-05", tier: 1 }
confidence: verified
derived_from: [C-02]
discovered_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:31:00Z
---
Stems that build, by requested RMS: -23 dB: 4 of 15; -28: 13; -30: 14; -33: 14; -36: 15. The rimshot has the highest crest (33.9 dB). Measured on dry float32 48 kHz renders of velocity 100 hits. The builder's cut and prepare functions are importable, so a planner can reuse its exact cutting. Wet or room-mixed clips, other velocities and real recordings will differ: compute the level per round (level_plan.py).
