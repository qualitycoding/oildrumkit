---
id: L-20261106T180400Z-scan-for-leaks-must-skip-audio-payload
title: A name-leak scan must skip raw audio sample bytes
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Low
tags: [domain:audio, tech:python, kind:tooling]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-06T18:04:00Z
---
## Trigger
A bundle verifier searches every file for reference stem names such as r05.
## What went wrong
The scan read WAV sample data, where a three-byte string occurs by chance, and failed a clean bundle (round 3).
## Correction
Strip the data chunk of WAV files before the scan (without_samples in verify_bundle.py).
## Prevention rule
Scan metadata and file names; never raw sample bytes.
## Detection check
Run the verifier on a bundle with random audio and short stem names.
## Evidence
Tools/listening/verify_bundle.py in qualitycoding/oildrumkit.
