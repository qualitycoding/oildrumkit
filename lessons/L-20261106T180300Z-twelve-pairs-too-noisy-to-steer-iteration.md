---
id: L-20261106T180300Z-twelve-pairs-too-noisy-to-steer-iteration
title: One 12-pair round is too noisy to steer small engine changes
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [domain:audio, phase:execution, kind:measurement]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:03:00Z
---
## Trigger
Iterating an engine on blind scores from a single 12-pair round with a fresh draw of events each time.
## What went wrong
Scores moved 6, 8, 12 of 12 across rounds with small changes. At 12 pairs, a listener who is right 70 % of the time scores anywhere from 6 to 12; the 12/12 after a change that sounded better said little about the change.
## Correction
Pool rounds, fix the event draw between rounds when comparing two versions, use more pairs, and take the listener's stated cue as the main evidence.
## Prevention rule
Do not read a change in score between rounds as an effect unless the draw is held fixed or at least 30 pairs are used.
## Detection check
Compute the binomial interval for the round score before claiming improvement.
## Evidence
listening/G-LISTEN/round-001 to round-006 in qualitycoding/oildrumkit.
