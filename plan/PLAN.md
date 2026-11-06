# Plan: oildrumkit listening phase (planning protocol v3.11, flag `audio`)

Rule: steps run in order; a step is done only when its checks print the expected output. Do not edit anything under `tests/`; do not regenerate
`tests/FROZEN_MANIFEST.sha256`.

## S-000 Self-initialise from the live repository
Goal: a clean working copy of `impl/listening-validation` containing current `main`, built and verified.
Set `WORK` to a new directory (for example `$HOME/work`; the working copy is `$WORK/oildrumkit`). Run: `bash plan/bootstrap/bootstrap.sh "$WORK"` then read `$WORK/bootstrap.json`.
Expected: exit 0; exit 0, `bootstrap.json` has `render` equal to `committed_render_sha256`'s match (`"render": "identical"`) and `code_changed_since_audited_commit` empty (otherwise stop and add a note to DECISIONS.md, because
the audited behaviour may differ). Exit 3 or 6 = BLOCKED: report to the owner, do nothing else. Exit 4 (vendored pair-tool hash differs) or 5 (render differs from the committed render) = stop and add a note to DECISIONS.md.
Also run `git push --dry-run origin impl/listening-validation` inside `$WORK/oildrumkit` and record the answer in `ENVIRONMENT.md` (A-005).

## S-010 Install the listening tools by test
Copy `stubs/Tools/listening/*` into `$WORK/oildrumkit/Tools/listening/`. From the bundle root run `OILDRUM_REPO=$WORK/oildrumkit python3 -m pytest tests -q -p no:cacheprovider` (the tests stay in the bundle and are not copied into the repository) and confirm exactly 20
failures and 3 passes (red check). Then implement each tool until the tests pass. The reference implementation in `research/reference-impl/` is
a worked solution; the implementer may copy it, but must still run the red check first and the green run afterwards.
Expected: `23 passed`. Commit the tools in the working copy: `listening: tools (S-010)`.

## S-020 Reference acquisition (opens G-REF)
`GATE-G-REF.md` already exists; do not rewrite it.
G-REF was answered `route-B` on 2026-11-05 (GATE-G-REF.RESPONSE.md); follow `assets/listening/ROUTE-B-GUIDE.md`.
On Route A/B, put the reference rows in `references/MANIFEST.json` in the working copy. Single hits that are cut from longer recordings carry `parent_file`, `parent_sha256`, `cut_start_s`, `cut_end_s`; `Tools/listening/cut_refs.py --manifest M --parents DIR --out SRC` rebuilds and hash-checks them, and `assets/listening/references/fetch_refs.py` downloads files that have a `raw_url`. Then run `ingest_refs.py --manifest M --src SRC --out REF`. Expected: it exits 0 and prints `ingested N references` (N>=12, at least 3 families). Anything else: return the output to
the owner; do not weaken the rules.
On Route C, skip to S-050 with the pair test marked `not-run`; no pass claim is possible (D-015).

## S-030 Candidates
`make_events.py --manifest M --f0 REF/f0.json --voicing Tools/voicing.json --out events.csv --drop-clamped` turns the manifest into an events CSV (pitch rule); remove the dropped stems from the reference directory. Build `render_events` with `g++ -O2 -std=c++17 -I Tools Tools/listening/render_events.cpp`, then `render_events --clips events.csv CAND_DIR` renders the candidates.
Expected: one candidate per reference, same stem; clamped pitches listed by name in the round record.

## S-035 Level and pairs
`level_plan.py --ref REF --cand CAND`, then `build_round.py --ref REF --cand CAND --out ROUND --round N --demos DEMOS --attribution FILE` (the attribution file must credit every CC BY reference and must not name stems), then `verify_bundle.py --round ROUND --ref REF`. Copy `f0.json`, `public_ok.json` and `REFERENCE_LINKS.md` from the ingest output into REF before building. Expected: `verify_bundle.py` exits 0 (level spread < 0.2 dB, no leak, reference correlation >= 0.99).

## S-040 Evidence and gate (opens G-LISTEN)
Render the nine demos with `render_events --timeline demo.csv demo.wav` into the demos directory passed to `build_round`. Hand the private `listener-bundle` (pairs, README, ANSWERS, demos) to the owner as files in the conversation. Push `evidence/G-LISTEN-round-N` (orphan) from `public-evidence/` only: it holds the pairs only if `public_ok.json` says every reference is redistributable, and always holds `REFERENCE_LINKS.md`, `PRIVATE_BUNDLE.sha256` and SHA256SUMS (D-020). Write
`GATE-G-LISTEN.md` with the metric table and the three questions. Stop.

## S-045 Score
After the answer: `record_round.py --score --out DIR --key archive/key.json.b64 --answers "A,B,..."`. Apply D-007. Pass -> S-050. Fail and round < 3 -> S-046. Round 3 fail -> escalate.

## S-046 Iterate on what was heard
Use the listener's words (skill step 7): compare the first 50-200 ms of correct pairs, log measured differences, one general change per difference,
re-run the repository's frozen metric checks (`Tools/metrics.py` and the tests listed in its README), new seed (D-006), rebuild, repeat S-035 onward.

## S-050 Record
`record_round.py --check --out DIR` (exit 0); commit the gate record with the response. Update `GATE-G-LISTEN` history.

## S-005 (before S-020; done 2026-11-06 by an independent reader, see `research/COLD_READ_2026-11-06.md`) Independent cold read
A reader with no access to this conversation runs `python3 plan/check_plan.py` and then reads `HANDOFF.md` and `PLAN.md` only; every question they
cannot answer is a defect to fix. Not done by the planner (D-018).

## S-070 Closing: `S-RETRO` then `S-KNOW`
Retrospective into `retro/` (question 5: sources checked after R7 and missing from the register, or listed with a licence not read from the source; add each to `knowledge/K-20261105T231000Z-source-register-oildrumkit-listening-phase-v2.md` in place until it is pushed). Lessons and knowledge items are in `lessons/` and `knowledge/`; create `GATE-G-003.md`; it names the register on its own line with its source count (32) and the number of rows with an unknown or unstated licence or origin (5). Push to the stores only on the owner's response.

## S-080 Final report. Nothing is pushed to the stores before the G-003 response.
