# Retrospective: oildrumkit listening phase

Run gen-20261105T211145Z-oildrumkit-listening-phase. Written 2026-11-06 after the owner stopped the listening tests.

## Outcome
The blind-listening gate G-LISTEN ran six rounds (three by default, three more on the owner's request, D-022 and D-023). Correct identifications: 11, 8, 9, 6, 8 and 12 of 12. The pass rule (at most 8 of 12 and no open qualitative finding) was met numerically in rounds 2, 4 and 5 and was never met together with the qualitative condition. The gate is closed as **failed**. The claim that the engine sounds like struck metal is not supported; the narrow claim in D-015 is not made.

## What was built
Listening tools and 23 frozen tests; a 16-recording reference set (13 used) with licences settled; six built, verified and delivered rounds; engine changes made in general ways (new `hfLoss` field, a float-overflow fix in the radiation law, voicing of bass, toms, cowbell, hats and cymbals). All frozen numeric targets were left as frozen: 2 of 4 pass (snare-roll correlation, HF/LF T60); partials at 50 ms (10/15) and centroid rise fail.

## What the listener heard
Round 1: timbre alone. Round 2: decay, brightness and resonance. Round 4: resonance. Round 5: more noise in cymbals, more resonance in drums. Round 6: harder, but all twelve still identified.

## Questions
1. Did the plan reach its goal? No: the gate failed. The evidence, tools and a list of audible cues exist.
2. Which assumptions were wrong? That one 12-pair round is a stable measure (swings of 6 to 12 across rounds with small changes); that the references resemble the target (they are propane and gas tanks and cymbals from a sample library, not oil drums); that the frozen HF/LF T60 target is compatible with realistic cymbal sustain.
3. Which defects did the tools have? Attribution default said "none required" for a CC BY set; the stem-leak scan read raw audio samples and gave a false positive; a missing `public_ok.json` silently left pairs out of the public evidence; the round cap had no owner-extension route until D-022. All fixed or recorded.
4. Which confounds were found? A lead-in timing cue on reference sides (fixed by cutting at the onset) and candidate tails that fell to digital silence (partly fixed; the closed-hat tail remained).
5. Sources checked after R7 and missing from the register, or listed with a licence not read from the source: rows 33 to 39 of register v3 were added. Rows 33, 35 and 36 carry licences taken from the owner, not read at the source (the sandbox could not reach Freesound). Rows 30 and 31 (Free to Use Sounds) were never read. Envato and catalogue pages looked at earlier in the session are not listed because the record was lost in a context reset.
6. What would be done differently? Run the reference-fit check before round 1 (compare onset, decay and brightness of references with candidates); decide at the start whether the frozen T60 target may be relaxed for cymbals; fix the pair draw across rounds so that score changes reflect the engine, and use more than 12 pairs; keep a running tooling log rather than reconstructing it.

## Open items
Independent pre-mortem (D-018); DAW and Windows validation; snares and rimshot with no reference; the owner's decision on the cymbal HF/LF T60 target; whether to repeat the test with true oil-drum recordings and a larger pair set.
