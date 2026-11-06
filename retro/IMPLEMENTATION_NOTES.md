# Implementation notes, run of 2026-11-06 (S-000 to S-040)

- S-000: bootstrap exit 0; push dry-run succeeds. S-010: red 19 failed / 3 passed, green 22 passed; tools committed at e94b84e.
- S-020: 16 references built into `references/MANIFEST.json` (7 steel-tank hits, 9 VCSL hits). `cut_refs.py` (new, not covered by a frozen test) rebuilds the cut files and checks all hashes (6/6). `ingest_refs.py` exit 0.
- Pitch rule: three VCSL hits (open hat 1432 Hz, anvil 1246 Hz, brake drum 1050 Hz) exceed the instrument range and were dropped (`--drop-clamped`); 13 references remain, none for the snares or the rimshot slot.
- Defect found: `build_round.py` writes "Attribution: none required" unless given a file, although a CC BY reference is in the set. A manifest-driven default would be safer. I passed `references/ATTRIBUTION.md`.
- Defect found: an attribution file that names reference stems makes `verify_bundle.py` fail (stems leak). Correct behaviour; the file now names sources only.
- Defect found: `build_round.py` needs `public_ok.json` in the reference directory, which `ingest_refs.py` writes beside the clips but my filtered directory lacked; without it the pairs were silently left out of the public evidence (fail closed, as designed).
- Round 1: seed 20261103, level -29 dB (worst crest 28.1 dB), `verify_bundle: ok`. Evidence branch `evidence/G-LISTEN-round-1` @ 507d06d (archive zip URL returns HTTP 200, 13.7 MB, so claim C-05 is verified for this branch). Gate: repo `GATE-G-LISTEN.md`.
- The repository already held a blocked round-001 record from an earlier plan; it was replaced with this round's record (`awaiting-listener`).
- Not done: S-045 onwards (waiting for the listener), S-070/S-080, an independent pre-mortem (D-018). The register is already pushed, so S-070 must publish register v3 as a new item superseding v2 (the plan text says "in place").

## Round 1 result and round 2 (2026-11-06)
- Round 1: 11/12 identified (fail). Listener: by timbre alone. A measured lead-in timing cue on five reference sides was fixed by re-cutting at the onset. Candidate tails also fell to digital silence.
- S-046 (D-021): engine and voicing changes at oildrumkit 46f0b85; round 2 built with seed 20262103, level -30 dB, `verify_bundle: ok`, evidence `evidence/G-LISTEN-round-2` @ ee1279d. Cue check of the round-2 pairs: openings and tails now comparable.
- Found on the way: the radiation law overflowed float (NaN) for plateModes above about 70 on drum-like voices; fixed.
- The frozen HF/LF T60 target (< 25 %) caps cymbal sustain at about 1.0-1.2 s against 1.7-2.6 s in the references: a decision for the owner if more sustain is wanted.
- Scratch tooling used (not committed): feature comparison and tuning scripts in the session scratchpad.

## Round 2 result and round 3 (2026-11-06)
- Round 2: 8/12 (p = 0.19; numeric condition met). Listener: the cues on the correct pairs were decay, brightness and resonance, so the qualitative condition was not met; owner chose `iterate`.
- Round 3 (the last allowed round): voicing changes at oildrumkit f3592fc for cowbell, closed hat and toms 2-8; seed 20263103, level -31 dB. Cymbals unchanged because the frozen HF/LF T60 target caps their sustain.
- Defect found and fixed in `verify_bundle.py` (oildrumkit 3 commits above f3592fc; synced to `research/reference-impl/`): the stem-name leak scan read raw audio samples, where a 3-byte name such as "r05" occurs by chance, so verification failed with no leak. The scan now skips WAV sample data. T-L03 in the frozen tests has the same style of scan but runs on small synthetic bundles.
- Known cue left in round 3: the closed-hat candidate decays to digital silence while its reference keeps a noise floor.
