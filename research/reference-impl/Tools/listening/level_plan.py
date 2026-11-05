#!/usr/bin/env python3
"""Choose the common RMS level for a round: min(-23, floor(-(highest crest factor over ALL clips on both sides) - margin)).
  level_plan.py --ref DIR --cand DIR [--seconds 1.0] [--fs 44100] [--margin-db 0.5]
Prints one line per clip and a final line `rms_db=<integer>`."""
import argparse, math, pathlib, sys
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from _common import CAP_DB, crest_db

def plan(ref, cand, seconds=1.0, fs=44100, margin=0.5):
    stems = sorted({p.stem for p in pathlib.Path(ref).glob("*.wav")} & {p.stem for p in pathlib.Path(cand).glob("*.wav")})
    if not stems:
        raise SystemExit("no matching stems")
    worst, rows = 0.0, []
    for s in stems:
        for side, d in (("ref", ref), ("cand", cand)):
            c = crest_db(pathlib.Path(d) / f"{s}.wav", seconds, fs)
            rows.append((s, side, c)); worst = max(worst, c)
    return min(CAP_DB, math.floor(-worst - margin)), worst, rows

def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", required=True); ap.add_argument("--cand", required=True)
    ap.add_argument("--seconds", type=float, default=1.0); ap.add_argument("--fs", type=int, default=44100)
    ap.add_argument("--margin-db", type=float, default=0.5)
    a = ap.parse_args(argv)
    level, worst, rows = plan(a.ref, a.cand, a.seconds, a.fs, a.margin_db)
    for s, side, c in rows:
        print(f"{s} {side} crest_db={c:.1f}")
    print(f"worst_crest_db={worst:.1f}")
    print(f"rms_db={level}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
