# Round 5 analysis

Answers (real side): a b a a a b b a b a a a. Score 8/12, one-sided p = 0.194 (chance 6/12). The numeric pass condition (at most 8 correct) is met, but two qualitative findings are open, so the gate does not pass.

Misses: pairs 1, 2, 8 and 10 (four of twelve). Round trend: 11, 8, 9, 6, 8 of 12; pooled rounds 2-5, 31/48.

Owner findings:
- Cymbals: the real recordings contain more noise than the synthesised clips.
- Drums: the real recordings have more resonance.

Change tried this round: splash and ride dampScale 0.12, hfLoss 0.6 (ring about 1.2 s). The owner heard the cymbals as too clean, so more ring did not close the gap; the missing element is broadband noise in the sustain. Drum resonance remains the discriminating cue, as in round 4.

Candidate next changes: raise the cymbal noise residual and shimmer (checking the frozen HF/LF T60 target, which noise decay also affects); lengthen low-mid mode decay for bass and toms (dampScale, hfLoss) without breaking the target; keep the closed-hat tail from falling to digital silence.
