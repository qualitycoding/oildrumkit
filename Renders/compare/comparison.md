# Before / after metrics

Single hits at v=1.0, dry, 48 kHz. `*` = window fell back to t=10 ms (signal < -50 dB by 50 ms).

| instrument | partials @50 ms (before -> after) | HF>2k energy per +velocity (dB, after) | T60 HF/LF (before -> after) | peak dBFS (before -> after) |
|---|---|---|---|---|
| Bass Drum | 4 -> 13 | +1.13 | - -> - | 5.7 -> -13.5 |
| Snare 1 | 3 -> 9 | -0.10 | 0.87 -> 0.21 | 4.3 -> -7.8 |
| Snare 2 | 2 -> 8 | -0.07 | 1.14 -> 0.15 | 4.5 -> -9.6 |
| Low Tom 1 | 4 -> 20 | +1.02 | - -> 0.03 | 4.8 -> -12.4 |
| Low Tom 2 | 4 -> 18 | +1.14 | - -> 0.03 | 4.8 -> -12.9 |
| Mid Tom 1 | 4 -> 20 | +1.61 | - -> 0.03 | 4.9 -> -10.5 |
| Mid Tom 2 | 4 -> 16 | +0.99 | - -> 0.03 | 4.8 -> -12.7 |
| High Tom 1 | 4 -> 17 | +1.33 | - -> 0.05 | 4.6 -> -11.4 |
| High Tom 2 | 4 -> 16 | +1.19 | - -> 0.05 | 4.5 -> -12.6 |
| Splash | 1 -> 6 | +1.20 | 0.74 -> 0.12 | -6.1 -> -20.6 |
| Ride | 3 -> 58 | +0.16 | 0.92 -> 0.12 | -6.3 -> -15.1 |
| Hi-Hat Closed | 1 -> 4 | +1.28 | 0.84 -> 0.12 | -9.8 -> -21.0 |
| Hi-Hat Open | 1 -> 28 | +1.19 | 0.88 -> 0.13 | -7.7 -> -17.2 |
| Cowbell | 4 -> 2 | +1.01 | - -> 0.11 | 1.3 -> -13.4 |
| Rimshot | 2 -> 5* | +0.54 | 1.60 -> 0.22 | 1.9 -> -13.9 |

## Snare roll repeatability (normalised cross-correlation of consecutive hits; lower = less machine-gun)

| band | before mean/max | after mean/max |
|---|---|---|
| full band | 0.657 / 0.884 | 0.310 / 0.474 |
| 100-1000 Hz | 0.928 / 0.994 | 0.518 / 0.815 |
| > 1 kHz | 0.081 / 0.150 | 0.304 / 0.488 |

## Phase 5 numeric targets (as written in the plan) -- reported honestly

1. Resolved partials >= 25 at 50 ms: **2/15 instruments meet it** (FAIL).
2. Centroid rise v0.2 -> v1.0 >= 400 Hz: **0/15 meet it** (FAIL); before was exactly 0 for all.
3. Successive snare-roll hits, full-band xcorr <= 0.97: max 0.474 (PASS); NOTE the legacy engine also passed this (max 0.884), so it does not discriminate -- see the 100-1000 Hz row.
4. HF (4-8 kHz) T60 < 25 % of LF T60: **14/14 measurable instruments meet it** (PASS).
