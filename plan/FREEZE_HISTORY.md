# Freeze history (planner amendments before hand-off)

Rule kept: after hand-off the implementer never edits `tests/` or regenerates `tests/FROZEN_MANIFEST.sha256`. Before hand-off the planner may unfreeze, edit, red-verify and re-freeze, and records it here.

| When | Why | Files changed | Old manifest sha256 (of the manifest file) | Red / green after |
|---|---|---|---|---|
| 2026-11-05 21:30 | First freeze | all tests, demos, bootstrap | n/a | 17 fail / 2 pass on stubs; 19 pass on the reference |
| 2026-11-05 21:55 | D-020: references need not be republished; licence rules relax and public evidence omits the pairs when any reference is not redistributable | tests/test_references.py (T-L08b, new T-L08d), tests/test_build_round.py (new T-L16) | e263a308d0a34feded9119c727b3383b8f658aa56087b679d49dc593722b3fb8 | 19 fail / 2 pass on stubs; 21 pass on the reference |
| 2026-11-06 11:40 | On-demand reference download (Rule 15 step d): added fetch_refs.py as a planner-supplied asset and its test | tests/test_fetch_refs.py (new T-L17) | 9cc87d94467ea0ab109922fdfd6832c4d0cdf5e67e0e47a3ab2f2138cecd5e9b | 19 fail / 3 pass on stubs; 22 pass on the reference |
| 2026-11-06 16:40 | D-022: the owner extended the gate beyond three rounds after round 3 failed; the round limit can be raised with OILDRUM_OWNER_EXTRA_ROUNDS | tests/test_build_round.py (new T-L18; T-L12 unchanged: default still refuses a fourth round) | 1585146e29dfe0bc5f38bc02608b13074b64a8314bc46083a5eec86dfe6adc21 | 20 fail / 3 pass on stubs; 23 pass on the reference |
