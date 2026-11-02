# PLAN — protocol v3.9 (constructed by ADDENDUM-PLAN-V3.9.md)

**Status:** constructed planning layout for a repository with no prior
protocol-based plan (`unversioned` baseline). Built from repository evidence
(18 source files, primary language C/C++).

## S-000 Intake and environment probe
Record environment class (ephemeral), durable-mount discipline, tool pins.

## Build and test steps
Derived from repository layout; authoritative execution venue is CI
(see `plan/CI-MATRIX.md`).

## S-REVIEW / S-ADVERSARIAL
Traditional and adversarial review steps before any packaging (recorded
substitution D-906 for this amendment; full reviews due before product work).

## S-LISTEN — blind listening realism gate (D-981)

Runs after the applicable review steps and before S-RETRO. Listening-only
authorization: prepare matched reference/candidate clips, build 12 onset
pairs with the vendored `scripts/make_blind_pairs.py` (frozen settings in
`plan/GATES.md` G-LISTEN), render the frozen demos, deliver the listener
bundle, and score with `scripts/score_blind.py` after answers are locked.
Record every round under `listening/G-LISTEN/round-00N/` and write
`GATE-G-LISTEN.md`; resume only after the human commits
`GATE-G-LISTEN.RESPONSE.md` with an allowed response. Round-001 is currently
blocked on missing realism references (see `listening/G-LISTEN/round-001/
result.json`); do not manufacture a score.

## Closing steps
- **S-RETRO** — retrospective over execution log and deviations.
- **S-KNOW** — lessons and knowledge items; G-003 controls publication to the
  `Lessons` and `knowledge` stores (do-not-push until sign-off).

The addendum is normative where the gap ledger says so; product execution is
not authorised by this document.
