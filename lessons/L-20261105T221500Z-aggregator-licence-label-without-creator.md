---
id: L-20261105T221500Z-aggregator-licence-label-without-creator
title: A CC0 label on an aggregator page with no named creator is not a usable grant
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [domain:audio, phase:research, kind:wrong-assumption]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A sound or sample page states 'CC0' but names no author and no upstream source, as on re-hosting sites.
## What went wrong
The best match for a real oil-drum recording was on Lots of Sounds with the line 'CC0: free for commercial use, no attribution required', no creator and no link to the original. A re-host cannot grant rights it does not hold, and a public evidence branch would redistribute the file.
## Correction
Recorded the page as a lead only, with permitted use 'read only until its origin is established' in the source register.
## Prevention rule
Treat a licence label as usable only when the page also names the creator or links the original release, and the licence is read there; otherwise list the source as a lead.
## Detection check
For each reference in the manifest, check that the manifest names an author and a source URL whose own page shows the same licence.
## Evidence
knowledge/K-20261105T223000Z-source-register-oildrumkit-listening-phase.md row 20.
