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
