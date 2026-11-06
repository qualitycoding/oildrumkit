#!/usr/bin/env python3
"""Round record helper.
  record_round.py --init  --round N --out DIR --candidate-commit SHA --branch NAME   -> DIR/result.json (status awaiting-listener)
  record_round.py --score --out DIR --key KEY.b64 --answers "A,B,..."                -> DIR/score.txt, result.json updated
  record_round.py --check --out DIR                                                  -> secret scan + schema keys; exit 1 on failure"""
import argparse, json, pathlib, subprocess, re, sys
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from _common import REPO, SECRET, seed_for

KEYS = ["schema_version", "repository", "branch", "candidate_commit", "gate_id", "round", "test_type", "status", "prerequisites",
        "frozen_metrics", "reference", "candidate_renders", "pair_builder", "evidence", "answers", "score", "qualitative_findings",
        "iterations", "response", "resume"]

def init(a):
    out = pathlib.Path(a.out); out.mkdir(parents=True, exist_ok=True)
    d = {"schema_version": 1, "repository": "qualitycoding/oildrumkit", "branch": a.branch, "candidate_commit": a.candidate_commit,
         "gate_id": "G-LISTEN", "round": a.round, "test_type": "realism", "status": "awaiting-listener", "prerequisites": [],
         "frozen_metrics": [], "reference": {"kind": "", "source_urls_or_ids": [], "licence": "", "checksums": ""},
         "candidate_renders": {"manifest": "", "checksums": ""},
         "pair_builder": {"script": "scripts/make_blind_pairs.py", "commit": "", "seed": seed_for(a.round), "pairs": 12, "window": "start",
                          "seconds": 1.0, "rms_db": None, "fade_seconds": 0.01, "sample_rate": 44100},
         "evidence": {"branch": "", "commit": "", "bundle": "", "sha256sums": ""},
         "answers": {"ordered": "", "submitted_at": "", "same_side_count": 0},
         "score": {"correct": 0, "total": 12, "one_sided_p": None, "pass_condition": "correct <= 8 && no unresolved qualitative finding"},
         "qualitative_findings": [], "iterations": [], "response": {"value": "", "recorded_at": "", "file": ""},
         "resume": {"next_step": "", "checkpoint_file": ".checkpoints/state.json", "checkpoint_commit": "", "execution_log_commit": ""}}
    (out / "result.json").write_text(json.dumps(d, indent=1) + "\n")
    return 0

def score(a):
    out = pathlib.Path(a.out); d = json.loads((out / "result.json").read_text())
    p = subprocess.run([sys.executable, str(REPO / "scripts" / "score_blind.py"), "--key", a.key, "--answers", a.answers], capture_output=True, text=True)
    if p.returncode != 0: print(p.stdout + p.stderr); return 1
    (out / "score.txt").write_text(p.stdout)
    m = re.search(r"(\d+)/(\d+) correct; chance [\d.]+; one-sided p = ([\d.]+)", p.stdout)
    picks = [s.strip().upper() for s in a.answers.replace(" ", ",").split(",") if s.strip()]
    d["score"].update(correct=int(m.group(1)), total=int(m.group(2)), one_sided_p=float(m.group(3)))
    d["answers"].update(ordered=a.answers, same_side_count=max(picks.count("A"), picks.count("B")))
    d["status"] = "scored"
    (out / "result.json").write_text(json.dumps(d, indent=1) + "\n")
    return 0

def check(a):
    out = pathlib.Path(a.out); bad = []
    d = json.loads((out / "result.json").read_text())
    bad += [f"result.json lacks {k}" for k in KEYS if k not in d]
    for f in out.rglob("*"):
        if f.is_file() and f.suffix in {".json", ".md", ".txt", ".csv"} and SECRET.search(f.read_text(errors="ignore")):
            bad.append(f"possible secret in {f.name}")
    for b in bad: print("FAIL:", b)
    return 1 if bad else 0

def main(argv=None):
    ap = argparse.ArgumentParser()
    g = ap.add_mutually_exclusive_group(required=True)
    for n in ("init", "score", "check"): g.add_argument("--" + n, action="store_true")
    ap.add_argument("--round", type=int); ap.add_argument("--out", required=True); ap.add_argument("--candidate-commit", default="")
    ap.add_argument("--branch", default=""); ap.add_argument("--key"); ap.add_argument("--answers")
    a = ap.parse_args(argv)
    return init(a) if a.init else score(a) if a.score else check(a)

if __name__ == "__main__":
    sys.exit(main())
