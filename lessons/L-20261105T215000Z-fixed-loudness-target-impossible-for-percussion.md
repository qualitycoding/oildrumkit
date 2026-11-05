---
id: L-20261105T215000Z-fixed-loudness-target-impossible-for-percussion
title: A fixed -23 dB RMS pair level cannot be met by percussive hits
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, phase:planning, kind:wrong-assumption]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A plan freezes a loudness target (here -23 dB RMS for blind pairs) for a gate tool that was written for sustained notes, and the material is percussive: short hits with a high peak-to-RMS ratio.
## What went wrong
The vendored pair builder stops with 'clip would exceed full scale' when a clip's peak would pass full scale at the target RMS. The 15 oildrumkit hits have a crest factor of 22 to 34 dB; at -23 dB only 4 of 15 stems built, at -36 dB all 15 did. A plan that froze -23 dB could not have produced a round.
## Correction
Measure the crest factor of the real clips first (spike S2), compute the level per round from the worst crest plus a margin, and record the change as a decision with a frozen test (D-002, T-L05).
## Prevention rule
Before freezing a loudness target for a gate tool, run the tool once on the actual material and record which stems fail; derive the level from the measured crest factor, not from the skill's default.
## Detection check
Run the pair builder at the frozen level on every candidate stem; any exit 1 with 'lower --rms-db' means the target is unattainable.
## Evidence
research/spikes/S2_RESULT.md in this run's generation branch; plan/DECISIONS.md D-002.
