# Plan: oildrumkit listening phase (planning protocol v3.11, flag `audio`)

Rule: steps run in order; a step is done only when its checks print the expected output. Do not edit anything under `tests/`; do not regenerate
`tests/FROZEN_MANIFEST.sha256`.

## S-000 Self-initialise from the live repository
Goal: a clean working copy of `impl/listening-validation` containing current `main`, built and verified.
Run: `bash plan/bootstrap/bootstrap.sh "$WORK"` then read `$WORK/bootstrap.json`.
Expected: exit 0; `bootstrap.json` has `ready:true`, `code_changed_since_audited_commit` empty (otherwise stop and add a note to DECISIONS.md, because
the audited behaviour may differ). Exit 3 or 6 = BLOCKED: report to the owner, do nothing else.
Also run `git push --dry-run origin impl/listening-validation` and record the answer in `ENVIRONMENT.md` (A-005).

## S-010 Install the listening tools by test
Copy `stubs/Tools/listening/*` into the working copy, run `OILDRUM_REPO=<copy> python3 -m pytest tests -q -p no:cacheprovider` and confirm exactly 17
failures and 2 passes (red check). Then implement each tool until the tests pass. The reference implementation in `research/reference-impl/` is
a worked solution; the implementer may copy it, but must still run the red check first and the green run afterwards.
Expected: `19 passed`. Commit: `listening: tools and tests (S-010)`.

## S-020 Reference acquisition (opens G-REF)
Write `GATE-G-REF.md` from `research/REFERENCE_LEADS.md` and `assets/listening/RECORDING-PROTOCOL.md`. Stop until the answer arrives.
On Route A/B, run `ingest_refs.py` on the owner's manifest. Expected: it exits 0 and prints `families>=3`. Anything else: return the output to
the owner; do not weaken the rules.
On Route C, skip to S-050 with the pair test marked `not-run`; no pass claim is possible (D-015).

## S-030 Candidates
`make_events.py` turns the reference manifest into an events CSV (pitch rule, `--drop-clamped`); `render_events --clips` renders the candidates.
Expected: one candidate per reference, same stem; clamped pitches listed by name in the round record.

## S-035 Level and pairs
`level_plan.py` then `build_round.py --round N`. Expected: `verify_bundle.py` exits 0 (level spread < 0.2 dB, no leak, reference correlation >= 0.99).

## S-040 Evidence and gate (opens G-LISTEN)
Render the nine demos with `render_events --timeline`. Push `evidence/G-LISTEN-round-N` (orphan; bundle + SHA256SUMS + ATTRIBUTION). Write
`GATE-G-LISTEN.md` with the metric table and the three questions. Stop.

## S-045 Score
After the answer: `record_round.py --score`. Apply D-007. Pass -> S-050. Fail and round < 3 -> S-046. Round 3 fail -> escalate.

## S-046 Iterate on what was heard
Use the listener's words (skill step 7): compare the first 50-200 ms of correct pairs, log measured differences, one general change per difference,
re-run the repository's frozen metric checks (`Tools/metrics.py` and the tests listed in its README), new seed (D-006), rebuild, repeat S-035 onward.

## S-050 Record
`record_round.py --check`; commit the gate record with the response. Update `GATE-G-LISTEN` history.

## S-005 (parallel, before S-020) Independent cold read
A reader with no access to this conversation runs `python3 plan/check_plan.py` and then reads `HANDOFF.md` and `PLAN.md` only; every question they
cannot answer is a defect to fix. Not done by the planner (D-018).

## S-070 Closing: `S-RETRO` then `S-KNOW`
Retrospective into `retro/` (question 5: sources checked after R7 and missing from the register, or listed with a licence not read from the source; add each to `knowledge/K-20261105T223000Z-source-register-oildrumkit-listening-phase.md` in place until it is pushed). Lessons and knowledge items are in `lessons/` and `knowledge/`; `GATE-G-003.md` names the register on its own line with its source count (32) and the number of rows with an unknown or unstated licence or origin (5). Push to the stores only on the owner's response.

## S-080 Final report. Nothing is pushed to the stores before the G-003 response.
