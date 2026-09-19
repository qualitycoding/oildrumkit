#!/usr/bin/env python3
"""calibrate.py -- iteratively set per-instrument outGain in voicing.json so the p90 single-hit peak
(v=1.0, dry, limiter off) hits TARGET * trim. Regenerates Source/Voicing.h each pass."""
import json, subprocess, os, sys
HERE = os.path.dirname(os.path.abspath(__file__)); ROOT = os.path.dirname(HERE)
TARGET = 0.5   # -6 dBFS
TRIM = {"Hi-Hat Closed": 0.7, "Hi-Hat Open": 0.7, "Splash": 0.8, "Ride": 0.8, "Cowbell": 0.8, "Rimshot": 0.9}
def run(cmd): return subprocess.run(cmd, shell=True, cwd=ROOT, capture_output=True, text=True, check=True).stdout
for it in range(12):
    run("python3 Tools/modes.py generate")
    run("g++ -O2 -std=c++17 Tools/calibrate.cpp -o /tmp/calibrate")
    out = [l.split() for l in run("/tmp/calibrate").strip().splitlines()]
    V = json.load(open(os.path.join(HERE, "voicing.json"))); worst = 0
    for (i, p90, mx), ins in zip(out, V["instruments"]):
        want = TARGET * TRIM.get(ins["name"], 1.0); cur = ins.get("outGain", V["defaults"]["outGain"])
        new = cur * want / float(p90); worst = max(worst, abs(want / float(p90) - 1))
        ins["outGain"] = float("%.5g" % new)
    json.dump(V, open(os.path.join(HERE, "voicing.json"), "w"), indent=1)
    print(f"pass {it}: worst deviation {worst*100:.1f}%")
    if worst < 0.02: break
run("python3 Tools/modes.py generate")
run("g++ -O2 -std=c++17 Tools/calibrate.cpp -o /tmp/calibrate")
print("final p90 / max peaks (linear):")
for (i, p90, mx), ins in zip([l.split() for l in run("/tmp/calibrate").strip().splitlines()], json.load(open(os.path.join(HERE, "voicing.json")))["instruments"]):
    print(f"  {ins['name']:14s} outGain {ins['outGain']:<10g} p90 {float(p90):.3f}  max {float(mx):.3f}")
