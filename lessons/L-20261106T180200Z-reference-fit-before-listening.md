---
id: L-20261106T180200Z-reference-fit-before-listening
title: Check how well the references match the target instrument before spending listening rounds
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, phase:execution, kind:wrong-assumption]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:02:00Z
---
## Trigger
A blind test compares synthesis with real recordings that are not of the same instrument (propane tanks and sample-library cymbals standing in for oil drums).
## What went wrong
Across six rounds the listener could separate the pairs by timbre because the references differed from the engine in bright tank ring and sustained noise, not necessarily because the engine was unrealistic. The first round alone scored 11/12.
## Correction
Compare onset, decay, brightness and noise floor of each reference with the candidate before the first round, and say in the plan which differences are reference mismatch.
## Prevention rule
Run a feature comparison (spectral centroid, band decay, tail level) between references and candidates before round 1; record the mismatches.
## Detection check
Any pair where reference and candidate centroid differ by more than a factor of two is a mismatch to log.
## Evidence
retro/RETROSPECTIVE.md; scores 11, 8, 9, 6, 8, 12 of 12.
