# GATE-G-LISTEN — struck-steel realism (blind listening)

**Status:** BLOCKED — realism references not secured (round-001 not scored).
**Branch:** `impl/listening-validation` (base `main` @ e31e8d041f49357ac6d87918a70831cf941971a1).
**Decision:** D-981 (plan/DECISIONS.md). **Frozen seed:** 20261103 (rounds +1000/+2000).

## Test type
Realism (candidate renders vs real struck-steel oil drum / sheet-steel
recordings). Legacy-versus-current renders are demos only, never references.

## Frozen settings
12 matched onset pairs, `--window start`, 1.0 s clips, common RMS −23 dBFS,
10 ms fades, 44.1 kHz PCM-24 WAV; pass condition ≤ 8/12 correct and no
unresolved qualitative finding; maximum three rounds; builder
`scripts/make_blind_pairs.py` (sha256 3c4f75854e2a…d6824c7) and scorer
`scripts/score_blind.py` (sha256 f2d9ea5c76a1…e49defa) vendored from
qualitycoding/skills @ 7915c7b7e0cecbee5116e72a4cf5699383df5983.

## Questions for the human listener
1. Which side is the real recording (per pair)?
2. Per demo: convincing metallic ring, attack, decay, velocity response, roll
   variation, cymbal shimmer vs hiss, snare rattle, hat choke, stereo/room
   character, cowbell partial structure?
3. Every adverse finding needs instrument and timestamp.

## Allowed responses
`proceed` → close G-LISTEN, resume **S-RETRO**, then the existing
S-KNOW/G-003 closing sequence.
`iterate: <notes>` → remain at G-LISTEN; general voicing/physics change in
`Tools/voicing.json` or engine mechanism; rerun metrics/reviews; new seed.
`proceed-with-rescope: <text>` → narrow the realism/instrument claim; update
README/research limitations before S-RETRO.
`stop` → halt at G-LISTEN with S-RETRO pending.

## Current state
- Round-001 record: `listening/G-LISTEN/round-001/result.json` (status
  `blocked`; prerequisite P-4 missing references).
- Reproducibility: current render bit-identical to committed
  `Renders/after` WAV (sha256 e487c3fb…387135d).
- Disclosed limitations (docs/RETROSPECTIVE.md) preserved and not converted to
  passes: 2/4 numeric targets, no DAW/Windows validation, open engineering
  items.
