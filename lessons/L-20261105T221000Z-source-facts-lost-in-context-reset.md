---
id: L-20261105T221000Z-source-facts-lost-in-context-reset
title: Source facts were lost across a context reset and a note understated what had been checked
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [phase:research, kind:process, applies:all]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A long planning run is compacted into a summary and the research notes are written afterwards from that summary.
## What went wrong
After the reset, the first reference-leads note said no specific file had been checked, although five sound pages had been opened with the web-fetch tool, one of them a close match. The error was found only when the sources were rebuilt from the session record for the register.
## Correction
Wrote the round log and the source register from the full session record, corrected the leads note, and disclosed that they were compiled afterwards (D-019).
## Prevention rule
Write each source into the round log's 'Sources checked' section as it is opened, with its outcome and licence, so that a context reset cannot erase it.
## Detection check
At every compaction point, compare the round log's source count with the number of fetch and search calls in the session record.
## Evidence
research/rounds/round-1.md, research/REFERENCE_LEADS.md and plan/DECISIONS.md D-019 in this run's generation branch.
