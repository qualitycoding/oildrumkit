---
id: L-20261105T220500Z-probe-reachability-and-reprobe-before-download-steps
title: Host reachability changes during a run and decides who can download
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [domain:audio, phase:planning, kind:environment]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A plan needs files from public sound libraries or archive URLs, and the sandbox reaches the web through a policy proxy.
## What went wrong
bigsoundbank.com gave no connection at 21:05 and answered 200 an hour later; github.com zip URLs were 403 until the repository was attached; freesound.org stayed 403 to the shell although its pages could be read with the web-fetch tool. A single early probe would have recorded a wrong picture.
## Correction
The plan probes in S-000 and falls back to a named human action (record, download in a browser, or demos only); spike S5 records each probe with its time.
## Prevention rule
Probe each host the plan depends on at the start of the step that needs it, record the time, and give every download a human fallback.
## Detection check
Re-run the probe list at the start of S-000 and S-020 and compare with ENVIRONMENT.md.
## Evidence
research/spikes/S5_RESULT.md in this run's generation branch.
