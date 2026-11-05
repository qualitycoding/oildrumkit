# Environment (pinned and verified 2026-11-05)

| Item | Pin | Existence / verification |
|---|---|---|
| Python | 3.13.16 (system) | `python3 --version` |
| numpy | 2.5.3 | PyPI `/pypi/numpy/2.5.3/json` -> 200 |
| scipy | 1.18.1 | PyPI -> 200 |
| soundfile | 0.14.0 | PyPI -> 200; install: `pip install --break-system-packages soundfile==0.14.0` |
| pytest | 9.2.0 | PyPI -> 200 |
| g++ | 13.3.0 (Ubuntu 24.04) | `g++ --version`; engine builds with `-O2 -std=c++17` (spike S1) |
| git | 2.43.0 | |
| Pair tools | `scripts/make_blind_pairs.py` sha256 3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7; `scripts/score_blind.py` sha256 f2d9ea5c76a731fd3dd553d698a9f1a92b393d272f6ea898e6a9e5cbde49defa | identical to `qualitycoding/skills` @ 108db63 `skills/audio-blind-listening-validation/scripts/` (checked 2026-11-05) |
| Target repo | `qualitycoding/oildrumkit` `main` 1900ac8; `impl/listening-validation` 21972a2 | `git ls-remote` |

Setup (verified commands):

```
python3 -m pip install --break-system-packages numpy==2.5.3 scipy==1.18.1 soundfile==0.14.0 pytest==9.2.0
bash plan/bootstrap/bootstrap.sh "$WORK"            # spike S3: exit 0 three times, same base commit each time
OILDRUM_REPO="$WORK/oildrumkit" python3 -m pytest tests -q -p no:cacheprovider      # run from the generation-branch checkout
```

Host reachability from the shell (spike S5, 2026-11-05; it changes during a run, so probe first): raw.githubusercontent.com 200; zenodo.org 200; pypi.org 200; bigsoundbank.com first 000 then 200; api.github.com 200 but
per-repository answers need the session to attach the repository (a 403 body says "GitHub access to this repository is not enabled for this
session"); github.com archive and codeload URLs 403 until the repository is attached, then 200 (32 MB zip); freesound.org 403; bigsoundbank.com no connection.

Not pinned (no tool needed): the listener's device and headphones (A-003).
