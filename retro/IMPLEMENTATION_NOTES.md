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
