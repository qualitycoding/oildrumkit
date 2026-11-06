# GATE-G-LISTEN — struck-steel realism (blind listening)

**Status:** ROUND 2 BUILT, AWAITING LISTENER. Round 1 scored 11/12 (fail; listening/G-LISTEN/round-001/analysis.md). Round 2: seed 20262103, level -30 dB, `verify_bundle: ok`, evidence branch `evidence/G-LISTEN-round-2` @ ee1279d.
**Branch:** `impl/listening-validation`; candidate commit 8ad2f125 (engine identical to `main` @ 1900ac8).
**Evidence:** orphan branch `evidence/G-LISTEN-round-1` @ 507d06d (pairs, demos, hashes, reference links; no key). Archive zip: https://github.com/qualitycoding/oildrumkit/archive/refs/heads/evidence/G-LISTEN-round-1.zip (HTTP 200 checked).
**Decision:** D-981 (plan/DECISIONS.md). **Seed:** 20261103 (rounds +1000/+2000). Record: `listening/G-LISTEN/round-001/result.json`.

## What the listener receives
`listener-bundle`: 12 pairs (`pair-NN-A/B.wav`, 1.0 s, 44.1 kHz, PCM-24, level -29 dB RMS), `README-LISTENER.txt`, a blank `ANSWERS.txt`, nine demo timelines (`demos/`), `ATTRIBUTION.md`. Answer with one letter per pair (the side you believe is the real recording), then the three questions below.

## Frozen settings
12 matched onset pairs, `--window start`, 1.0 s clips, 10 ms fades; RMS -29 dB (computed by `level_plan.py`; the frozen -23 dB aborts because these percussion hits have a worst crest factor of 28.1 dB, D-002); pass condition ≤ 8/12 correct and no unresolved qualitative finding; at most three rounds; builder sha256 3c4f75854e2a…d6824c7, scorer f2d9ea5c76a7…e49defa.

## Metric table (recorded, not re-run; the render is bit-identical to `Renders/after`)
| Target | Result |
|---|---|
| Resolved partials ≥ 25 at 50 ms | 2/15 instruments — FAIL |
| Centroid rise v0.2→v1.0 ≥ 400 Hz | 0/15 — FAIL |
| Snare-roll full-band xcorr ≤ 0.97 | max 0.474 — PASS (does not discriminate; legacy also passed) |
| HF (4–8 kHz) T60 < 25 % of LF T60 | 14/14 measurable — PASS |

## References (13 used of 16 ingested)
Licences: CC0 (VCSL; klakmart and neirbonad1 owner-attested) and CC BY 4.0 (betchkal, credited). Full list with hashes: `listening/G-LISTEN/round-001/REFERENCE_LINKS.md`. No reference audio is stored in this repository.
Limits of what a pass can show:
- Bass and toms (7 refs) are steel tanks from three recordings; snares and the rimshot slot have no real reference and are not in the scored pairs. Three references (open hat, anvil, brake drum) were dropped because their pitch is outside the instrument's range.
- A pass supports only the narrow claim in D-015, not general realism.

## Questions for the listener
1. Which side is the real recording (per pair)?
2. Per demo: is the metallic ring, attack, decay, velocity response, roll variation, cymbal shimmer (not hiss), hat choke, stereo/room character and cowbell partial structure convincing? Name the demo and the time of any problem.
3. Did you tell the pairs apart by the metal itself, or by room sound, noise or loudness? Which pairs? Every adverse finding needs instrument and timestamp.

## Allowed responses
`proceed` → close G-LISTEN, resume S-RETRO, then the S-KNOW/G-003 sequence.
`iterate: <notes>` → remain at G-LISTEN; general voicing/physics change in `Tools/voicing.json` or engine mechanism; rerun metrics/reviews; new seed (D-006).
`proceed-with-rescope: <text>` → narrow the realism/instrument claim; update README/research limitations before S-RETRO.
`stop` → halt at G-LISTEN with S-RETRO pending.

## Disclosed limitations preserved
2/4 numeric targets, no DAW/Windows validation, open engineering items (docs/RETROSPECTIVE.md).

## Round 2 changes (S-046)
General engine changes from the round-1 measurements: new recipe field `hfLoss` (frequency-dependent loss, default unchanged); overflow fix in the radiation law; shorter contact time and lower damping on bass, toms, hats and cowbell; longer sustain on splash and ride within the frozen HF/LF T60 target; references re-cut at the onset. Snares and the rimshot are unchanged (no reference). Numeric targets: partials 11/15 (fail, was 2/15), centroid rise 0/15 (fail), snare roll pass, HF/LF T60 pass (max 0.22). Nothing relaxed. Record: `listening/G-LISTEN/round-002/result.json`.
