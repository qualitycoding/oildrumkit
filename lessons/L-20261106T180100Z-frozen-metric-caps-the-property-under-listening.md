---
id: L-20261106T180100Z-frozen-metric-caps-the-property-under-listening
title: A frozen numeric target can cap the very property a listening test judges
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, phase:implementation, kind:wrong-assumption]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:01:00Z
---
## Trigger
A gate has frozen numeric targets (here HF/LF T60 below 25 %) and a blind test that judges realism.
## What went wrong
Cymbal sustain stayed at 1.0 to 1.2 s against 1.7 to 2.6 s in the references. The listener named resonance and noise in the cymbals in rounds 4 and 5. The target could not be relaxed by the implementer, so several rounds went on workarounds (hfLoss, residual noise).
## Correction
Check at planning time whether each frozen target limits a property the listener will judge, and ask the owner early whether it may move; trial each voicing change against the target before building a round.
## Prevention rule
For every frozen numeric target, name the audible property it constrains and the reference value; if the reference breaks the target, escalate before round 1.
## Detection check
Compare the reference clips' value of the target quantity with the threshold before round 1.
## Evidence
retro/RETROSPECTIVE.md; listening/G-LISTEN/round-004 and round-005 analysis in qualitycoding/oildrumkit.
