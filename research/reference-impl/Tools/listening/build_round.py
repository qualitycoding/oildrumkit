#!/usr/bin/env python3
"""Build and verify one listening round.
  build_round.py --ref REF_DIR --cand CAND_DIR --out ROUND_DIR --round N [--attribution FILE] [--demos DIR]
  build_round.py --print-seed --round N
Writes ROUND_DIR/listener-bundle (what the listener sees) and ROUND_DIR/archive (key, mapping, settings; never shown before scoring)."""
import argparse, base64, hashlib, json, pathlib, shutil, subprocess, sys, tempfile
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from _common import REPO, seed_for
import level_plan, verify_bundle

README = """Listening test, round {rnd}

Each pair-NN has two clips, A and B, about one second long. One of them is a recording of a real struck steel drum, barrel or
sheet. The other is not. Play them in order, A then B, once each, on your own headphones in a quiet room, without changing the volume.
For each pair write A or B in ANSWERS.txt: the side you believe is the real recording. Then answer the questions at the end.
Do not look at any file other than these. Send the answers back before anything else is shown to you.

Questions:
1. For each demo (if included): is the metallic ring, attack, decay and variation convincing? Name the demo and the time of any problem.
2. Did you tell the pairs apart by the metal itself, or by room sound, background noise or loudness? Say which pairs.
"""

def main(argv=None):
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref"); ap.add_argument("--cand"); ap.add_argument("--out")
    ap.add_argument("--round", type=int, required=True); ap.add_argument("--print-seed", action="store_true")
    ap.add_argument("--attribution"); ap.add_argument("--demos")
    a = ap.parse_args(argv)
    seed = seed_for(a.round)
    if a.print_seed:
        print(seed); return 0
    out = pathlib.Path(a.out)
    lb, arc = out / "listener-bundle", out / "archive"
    if out.exists() and any(out.iterdir()): raise SystemExit(f"{out} is not empty: a round directory is never overwritten")
    lb.mkdir(parents=True); arc.mkdir(parents=True)
    level, worst, _ = level_plan.plan(a.ref, a.cand)
    with tempfile.TemporaryDirectory() as t:
        cmd = [sys.executable, str(REPO / "scripts" / "make_blind_pairs.py"), "--ref", a.ref, "--cand", a.cand, "--out", t,
               "--seed", str(seed), "--pairs", "12", "--window", "start", "--seconds", "1.0", "--rms-db", str(level),
               "--fade", "0.010", "--fs", "44100"]
        p = subprocess.run(cmd, capture_output=True, text=True)
        if p.returncode != 0: raise SystemExit("builder failed: " + p.stdout + p.stderr)
        for f in pathlib.Path(t).glob("pair-*.wav"): shutil.copy2(f, lb / f.name)
        for n in ("key.json.b64", "sources.json.b64"): shutil.copy2(pathlib.Path(t) / n, arc / n)
    (lb / "README-LISTENER.txt").write_text(README.format(rnd=a.round))
    (lb / "ANSWERS.txt").write_text("".join(f"pair-{i:02d}:\n" for i in range(1, 13)) + "\nDemo and weak-spot notes (instrument and time):\n")
    (lb / "ATTRIBUTION.md").write_text(pathlib.Path(a.attribution).read_text() if a.attribution else "Attribution: none required for this round.\n")
    if a.demos:
        shutil.copytree(a.demos, lb / "demos")
    sums = "".join(f"{hashlib.sha256(f.read_bytes()).hexdigest()}  {f.relative_to(lb).as_posix()}\n"
                   for f in sorted(lb.rglob("*")) if f.is_file() and f.name != "SHA256SUMS")
    (lb / "SHA256SUMS").write_text(sums)
    settings = {"round": a.round, "seed": seed, "pairs": 12, "window": "start", "seconds": 1.0, "rms_db": level, "worst_crest_db": round(worst, 2),
                "fade_seconds": 0.01, "sample_rate": 44100,
                "builder_sha256": hashlib.sha256((REPO / "scripts" / "make_blind_pairs.py").read_bytes()).hexdigest()}
    (arc / "settings.json").write_text(json.dumps(settings, indent=1) + "\n")
    bad = verify_bundle.verify(out, a.ref)
    for b in bad: print("FAIL:", b)
    print(f"round {a.round}: seed {seed}, rms_db {level}, verify {'FAILED' if bad else 'ok'}")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
