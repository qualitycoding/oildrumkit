---
id: L-20261105T220000Z-frozen-tests-must-fail-against-stubs
title: Frozen tests that pass against a stub prove nothing
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [tech:pytest, phase:verification, kind:verification-gap]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A plan freezes tests before the implementation exists and checks 'red' only by asking whether the suite fails overall.
## What went wrong
Several tests passed against 'NOT IMPLEMENTED' stubs: one asserted on an exit code that stubs also returned, another checked that a function refused bad input and the stub refused everything. They would have passed against any implementation, or none.
## Correction
Each test was strengthened until a known-good implementation passed and the stub failed; the red run is recorded by count (17 fail, 2 pass by design) and the two passing tests are named.
## Prevention rule
Run the whole frozen suite against stubs and list every test that passes; each must be one that is meant to pass before implementation, or it is rewritten.
## Detection check
pytest against the stub tree prints the failing count; compare it to the count the plan states.
## Evidence
tests/ and plan/PLAN.md S-010 in this run's generation branch.
