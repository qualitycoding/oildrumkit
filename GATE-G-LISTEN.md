# GATE-G-LISTEN — struck-steel realism (blind listening)

**Status:** CLOSED, FAILED after six rounds (11, 8, 9, 6, 8, 12 of 12; owner-extended, D-022 and D-023). The owner stopped the listening tests after round 6. Open qualitative findings: cymbal noise, drum resonance; round 6 cue not stated. Numeric targets not relaxed (2 of 4 pass). The claim that the engine sounds like struck metal is not supported.
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

## Round 3 changes (S-046, from the round-2 findings)
Cowbell: 40 plate modes, shorter contact, more damping. Closed hat: shorter contact, less frequency-dependent loss, more scrape, more damping. Toms 2-8: damping x0.6 again and longer contact (darker, longer). Cymbals, bass, low tom 1, snares and rimshot unchanged; cymbal sustain stays capped by the frozen HF/LF T60 target (not relaxed). Numeric targets: partials 11/15 (fail), centroid rise 0/15 (fail), snare roll pass (0.405), HF/LF T60 pass (0.219). Tool fix: `verify_bundle.py` no longer matches stem names inside audio samples. Record: `listening/G-LISTEN/round-003/result.json`.

## Round 4 (owner extension, D-022)
After round 3 failed (9/12) the owner extended the gate (2026-11-06 16:16), remarking that the pairs he failed to identify were quite different in timbre from the synthesised clips. Changes: bass, low tom 1, cowbell, closed hat and toms 2-4 moved closer to their references (spectral balance and decay); cymbals unchanged because the frozen HF/LF T60 target caps their sustain (not relaxed). Numeric targets: partials 10/15 (fail), centroid rise 0/15 (fail), snare roll pass (0.453), HF/LF T60 pass (0.219, all 15 measurable). The round limit is raised with `OILDRUM_OWNER_EXTRA_ROUNDS=1`. Record: `listening/G-LISTEN/round-004/result.json`.

## Round 5 (second owner extension)
After round 4 scored 6/12 with resonance as the discriminating factor, the owner asked for another round. Change: splash and ride get lower damping (dampScale 0.12) with hfLoss 0.6, giving a longer ring (20 dB decay about 1.2 s) while the HF/LF T60 ratio stays at 0.171 and 0.185 (limit 0.25; hfLoss 0.35 failed at 0.28, so it was not used). Numeric targets: partials 10/15 (fail), centroid rise 0 measured (fail), snare roll pass (0.485), HF/LF T60 pass (0.219, all 15 measurable). The round limit is raised with `OILDRUM_OWNER_EXTRA_ROUNDS=2`. Record: `listening/G-LISTEN/round-005/result.json`.

## Round 6 (third owner extension)
After round 5 (8/12) the owner reported more noise in the real cymbals and more resonance in the real drums. Changes: splash and ride hfLoss 0.4, residual 0.9, scrape 0.8; toms hfLoss 0.5 and cavT60 0.5 (toms 4-6). Numeric targets: partials 10/15 (fail), centroid rise 0 measured (fail), snare roll pass (0.433), HF/LF T60 pass (0.219, all 15 measurable). `OILDRUM_OWNER_EXTRA_ROUNDS=3`. Record: `listening/G-LISTEN/round-006/result.json`.

## Closure
The owner stopped the listening tests on 2026-11-06 after round 6 (12/12, p 0.0002), asking to continue the plan implementation. The gate record is final: failed. Rounds 2 to 6 pooled 43/60. Retrospective and decisions are in the plan bundle (`retro/RETROSPECTIVE.md`, D-023). Open items: cymbal HF/LF T60 target (owner's decision), snares and rimshot never compared with a reference, references are tanks and sample-library cymbals rather than oil drums.
