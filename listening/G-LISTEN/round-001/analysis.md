# Round 1 analysis (scored 2026-11-06 14:2x)

Answers: B,A,B,A,A,A,A,A,B,A,A,B. Score: 11/12 correct (one-sided p = 0.0032). Pass needs ≤ 8/12, so round 1 fails (D-007). The only miss was pair-06 (r11, VCSL splash).

## Measured cues in the pair files (opening and tail levels, dB RMS, mono)
| Pair | Source | Reference side | Reference level 0-20 ms | Candidate level 0-20 ms | Reference 0.5-1 s | Candidate 0.5-1 s |
|---|---|---|---|---|---|---|
| 01 | r02 betchkal | B | -70 | -16 | -47 | -50 |
| 02 | r03 betchkal | A | -61 | -16 | -42 | -47 |
| 07 | r01 betchkal | A | -83 | -16 | -61 | -51 |
| 08 | r05 klakmart | A | -58 | -16 | -34 | -46 |
| 10 | r07 neirbonad1 | A | -57 | -15 | -35 | -53 |
| 03, 09, 12 | r13, r08, r14 | B | -12 to -17 | -13 | -68 to -75 | -108 to -120 |
| 04, 06, 11 | r12, r11, r10 | A, B, A | about -13 to -33 | -13 to -17 | -30 to -32 | -50 to -114 |

1. **Leading silence on five reference sides (pairs 01, 02, 07, 08, 10).** The reference clip is near-silent for the first 20-50 ms, then the hit starts; every candidate starts at full level at sample 0. The pair builder with `--window start` takes the start of the file and does not align onsets. The cut files were made with a 50 ms lead-in (and the neirbonad1 file has its own lead-in), so these five pairs are identifiable by timing alone. This is a defect in my reference preparation, not a property of the engine.
2. **Digital silence in the candidate tails.** In pairs 03, 04, 06, 09 and 12 the candidate decays to -108 dB or lower within a second, while the real recordings keep a noise floor or ring at -30 to -75 dB. A listener can use that without judging the metal. For cymbals and splashes (pairs 04, 06, 11) the real hits also ring far longer (-30 dB at 0.5-1 s against -50 to -114 dB for the candidate): a real difference in sustain, probably worth a voicing change.
3. Pair 06 (the one miss): the candidate was side A and the reference side B; the candidate there decays quickly while the reference rings on.

## Status
The listener's own words (how the pairs were told apart, demo findings) have not been given yet. Without them S-046 cannot name the differences heard. Item 1 should be fixed in the pair construction before round 2, so the next round tests the sound and not the timing.

## Listener's report (2026-11-06 14:22)
"I identified them purely by timbre." The lead-in timing cue (item 1 above) therefore did not decide the answers; the round counts as a genuine fail. No demo notes were given.

## Timbre differences measured (first 150 ms after the peak, reference against candidate)
- Brightness: energy above 2 kHz is 20-80 dB lower in the candidate. Centroid: tank hits 2.6-3.7 kHz against 0.3-0.8 kHz; cowbell 2.6-2.9 against 1.1 kHz; hat 8.6 against 0.7; splash 5.6 against 1.0. The klakmart toms (pairs 05, 08) are close in centroid (0.6-0.8 kHz against 0.7-0.8 kHz).
- Decay (time to fall 20 dB): toms 430-660 ms against 146-168 ms; splash about 990 ms against about 40 ms; ride 647 against 183 ms; cowbell 85-116 against 57-59 ms; bass roughly equal (72-194 against 120-129 ms).
- Validity caveat: the bass and tom references are propane and gas tanks (thin steel, high ring), not 55-gallon oil drums, and the cymbal references are VCSL suspended cymbals; matching them moves the instrument towards those objects.

## First experiment (scratch copy, not committed)
Global `tauScale` 0.35 (shorter contact pulse) and `dampScale` x0.3 for toms (3-7), x0.05 splash, x0.3 ride, x0.5 closed hat and cowbell. Result: toms and cowbell much closer in brightness (toms now brighter than the klakmart toms); tom decay 300-375 ms against 430-660 ms; splash and ride still far too short (about 215-480 ms against more than 1700 ms) because the cymbal tail is the noise `residual`, not the modes, so it needs a mechanism change; bass unchanged (centroid 0.3 kHz against 2.8-3.6 kHz, because bass has almost no energy above 2 kHz at 36-38 Hz). Numeric target 1 (partials >= 25 at 50 ms) rose from 2/15 to 8/15; target 4 (HF/LF T60 < 25 %) still passes for every measurable voice (largest ratio 0.206). No target was relaxed. The committed `Renders/after` would no longer be reproducible after such a change, and the plan's bootstrap check for a changed engine would trip.
