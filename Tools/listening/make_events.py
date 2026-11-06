#!/usr/bin/env python3
"""Turn the reference manifest into a candidate event file (decision D-006). Setup parameters only; no model constants change.
  make_events.py --manifest MANIFEST.json --f0 F0.json --voicing Tools/voicing.json --out EVENTS.csv [--drop-clamped]
Pitch = row `pitch_hz` (owner-measured with a tuner) if present, else the f0 estimate from ingest. A pitch outside the instrument's
range would be clamped by the engine: such rows are reported on stderr as `pitch_clamped <stem> <asked> <used>` and, with
--drop-clamped, left out of the event file (their stems must then also be left out of ref/ and cand/)."""
import argparse, json, pathlib, sys

HEAD = "stem,time_s,instrument,velocity,hammer,strike,damping,room,pitch_hz,seed,duration_s"

def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--manifest", required=True); ap.add_argument("--f0", required=True); ap.add_argument("--voicing", required=True)
    ap.add_argument("--out", required=True); ap.add_argument("--drop-clamped", action="store_true")
    a = ap.parse_args(argv)
    rows = json.loads(pathlib.Path(a.manifest).read_text())["references"]
    f0 = json.loads(pathlib.Path(a.f0).read_text())
    inst = json.loads(pathlib.Path(a.voicing).read_text())["instruments"]
    lines, kept = [HEAD], 0
    for r in rows:
        pitch = float(r.get("pitch_hz") or f0[r["stem"]])
        lo, hi = float(inst[r["instrument"]]["minHz"]), float(inst[r["instrument"]]["maxHz"])
        used = min(max(pitch, lo), hi)
        if used != pitch:
            print(f"pitch_clamped {r['stem']} {pitch:.1f} {used:.1f}", file=sys.stderr)
            if a.drop_clamped: continue
        lines.append(",".join([r["stem"], "0", str(r["instrument"]), f"{float(r['velocity']):.2f}", r.get("hammer", "wood"),
                               f"{float(r.get('strike', 0.5)):.2f}", "1.0", "0.0", f"{pitch:.1f}", "", "3.0"]))
        kept += 1
    pathlib.Path(a.out).write_text("\n".join(lines) + "\n")
    print(f"{kept} of {len(rows)} rows written to {a.out}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
