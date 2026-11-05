# Risk register (single-author pre-mortem; not independent, see D-018)

Imagine the run failed. Most likely causes, in order:

| ID | Failure | Mitigation in plan |
|---|---|---|
| R-1 | No references arrive, so no scored test ever runs. | G-REF with Routes A/B/C; Route C is honest about the missing pass claim. |
| R-2 | References are non-redistributable and are published in a public bundle. | D-014; `ingest_refs.py` refuses NC/ND/unknown (T-L08). |
| R-3 | A reference bundle clip is silently noise or the wrong note. | T-L02 independent decode, tested to fail. |
| R-4 | The builder aborts at -23 dB and the implementer lowers it ad hoc. | D-002 and `level_plan.py` (T-L05). |
| R-5 | Listener hears a loudness or format difference, not realism. | Level spread < 0.2 dB (verify_bundle), same PCM-24 format. |
| R-6 | A clamped pitch makes candidate and reference differ in note. | `make_events.py --drop-clamped`; clamped stems are listed in the round record (T-L13, T-L14). |
| R-7 | The sandbox is reset mid-run and work is lost. | Idempotent bootstrap; commit and push each step; A-004. |
| R-8 | Key leaks through file names or the listener bundle. | T-L03; key base64 and outside the bundle. |
| R-9 | Iteration overfits to the 12 clips. | Skill step 7.5; new seed each round; at most 3 rounds. |
| R-10 | A pass is over-read as "sounds real". | D-015 limits the claim. |
