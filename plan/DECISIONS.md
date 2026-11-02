
## Addendum v3.9 decisions (2026-11-02)

- **D-900** — Planning-only amendment. Product scope, public interfaces,
  expected test behaviour, performance targets and human gates do not change,
  except the mandatory split G-003 lesson/knowledge store semantics.
- **D-901** — Source-branch drift: the planning source branch tip `86af6a7b4cc1`
  is the rollout phase's `plan: add protocol v3.9 addendum` commit, a direct
  child of the audited source commit `35fbfead61a1`. The implementation
  branch descends from `86af6a7b4cc1`, which descends from the audited commit;
  no newer planning was overwritten or rebased away.
- **D-902** — No platform expansion: no new OS/architecture/runtime support is
  claimed; CI coverage is recorded as plan-level jobs/waivers only.
- **D-903** — No release, deployment or package publication is authorised or
  planned by this amendment.
- **D-904** — Ambiguities resolved from existing plan artifacts and repository
  evidence only; defaults follow the addendum's no-added-platform / explicit
  waiver rule.
- **D-905** — Local build and test execution is not available in the ephemeral
  sandbox; the CI matrix jobs are the authoritative execution venue (honest
  substitution; see DEVIATIONS.md).
- **D-906** — Fresh-context independent review is unavailable in this
  environment; the cold read is recorded as a substitution with High risk
  (S-ADD-090), not claimed as an independent pass.
- **D-980** — Mutation testing design: see `reviews/MUTATION.md` (priced
  three-tier trigger strategy, one aggregator job, report-based score gate).
- **D-981** — Listening-only amendment (blind realism gate G-LISTEN):
  branch `impl/listening-validation` created from current `main`
  (e31e8d041f49357ac6d87918a70831cf941971a1); `audio = true` set in
  plan/PROFILE.md and checkpoint mode flags; S-LISTEN added after the
  applicable reviews and before S-RETRO; G-LISTEN defined in plan/GATES.md
  with 12 onset pairs, `--window start`, 1.0 s clips, −23 dBFS RMS, 10 ms
  fades, 44.1 kHz PCM-24, ≤8/12 pass, max three rounds, base seed 20261103.
  Vendored canonical pair tools from qualitycoding/skills @
  7915c7b7e0cecbee5116e72a4cf5699383df5983:
  scripts/make_blind_pairs.py sha256
  3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7;
  scripts/score_blind.py sha256
  f2d9ea5c76a731fd3dd553d698a9f1a92b393d272f6ea898e6a9e5cbde49defa.
  Round-001 is blocked on missing redistribution-compatible struck-steel
  references (recorded honestly in listening/G-LISTEN/round-001/result.json).
  The retrospective's disclosed limitations (2/4 numeric targets, no
  DAW/Windows validation, open engineering items) are preserved; no frozen
  test was weakened; product execution remains unauthorised by this amendment.
