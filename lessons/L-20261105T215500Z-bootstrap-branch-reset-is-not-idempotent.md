---
id: L-20261105T215500Z-bootstrap-branch-reset-is-not-idempotent
title: A bootstrap that resets the implementation branch can discard unpushed work
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:git, phase:planning, kind:tooling]
recorded_by: planner
run: gen-20261105T211145Z-oildrumkit-listening-phase
recorded_at: 2026-11-05T22:40:00Z
---
## Trigger
A self-initialising script re-runs after an interruption in a working copy that already holds local commits on the implementation branch.
## What went wrong
The first bootstrap used 'git checkout -B <branch> origin/<branch>', which moves the local branch to the remote tip and drops local commits that were not pushed. The sandbox is ephemeral, but a re-run inside the same session would still have lost work.
## Correction
The script now resumes the local branch, fast-forwards only, merges origin/main without resetting, and exits 6 (BLOCKED) when local and remote have diverged. Three consecutive runs gave the same base commit.
## Prevention rule
A bootstrap or resume script must never reset, force-checkout or delete a branch; it fast-forwards or stops with a distinct exit code.
## Detection check
Run the script three times in one directory, with a local-only commit made between runs; the commit must still be there and the exit code 0 or 6.
## Evidence
plan/bootstrap/bootstrap.sh and research/spikes/S3_RESULT.md in this run's generation branch.
