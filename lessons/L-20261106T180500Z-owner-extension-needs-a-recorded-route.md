---
id: L-20261106T180500Z-owner-extension-needs-a-recorded-route
title: Give the round cap an owner-controlled extension route and record it
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Low
tags: [domain:process, kind:tool-gap]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:05:00Z
---
## Trigger
A tool refuses a fourth round because the plan froze a three-round cap.
## What went wrong
The owner wanted more rounds after round 3; the tool refused and the frozen tests forbade edits.
## Correction
Add an owner-declared extension (OILDRUM_OWNER_EXTRA_ROUNDS), a frozen test (T-L18), and record each extension as a decision (D-022, D-023).
## Prevention rule
When freezing a cap on rounds, freeze the extension route with it and require each use to be recorded.
## Detection check
Try to build round cap+1 with and without the extension variable.
## Evidence
plan/DECISIONS.md D-022 and D-023; tests/test_build_round.py T-L18.
