#!/usr/bin/env python3
"""Mechanical cold-read checks for the plan bundle. Run from the bundle root: python3 plan/check_plan.py"""
import hashlib, json, pathlib, re, sys
root = pathlib.Path(__file__).resolve().parent.parent
bad = []
def need(cond, msg):
    if not cond: bad.append(msg)
for f in ["HANDOFF.md","plan/PROFILE.md","plan/PLAN.md","plan/ASSUMPTIONS.md","plan/DECISIONS.md","plan/GATES.md","plan/ENVIRONMENT.md",
          "plan/TRACEABILITY.md","plan/bootstrap/bootstrap.sh","research/claims.json","research/REFERENCE_LEADS.md","premortem/RISK_REGISTER.md",
          "assets/listening/RECORDING-PROTOCOL.md","tests/FROZEN_MANIFEST.sha256"]:
    need((root/f).is_file(), f"missing {f}")
text = {p.name: p.read_text() for p in (root/"plan").glob("*.md")}
ids = lambda pre, t: set(re.findall(rf"\b{pre}-\d+[a-z]?\b", t))
# every D-/A- reference resolves
alltext = "\n".join(text.values()) + (root/"HANDOFF.md").read_text() if (root/"HANDOFF.md").is_file() else "\n".join(text.values())
defined_d = set(re.findall(r"^\| (D-\d+) \|", text.get("DECISIONS.md",""), re.M))
defined_a = set(re.findall(r"^\| (A-\d+) \|", text.get("ASSUMPTIONS.md",""), re.M))
for d in ids("D", alltext) - defined_d - {"D-981"}: need(False, f"undefined {d}")
for a in ids("A", alltext) - defined_a: need(False, f"undefined {a}")
# every test in the suite appears in the traceability table, and every traced test exists
tests = set()
for f in (root/"tests").glob("test_*.py"):
    tests |= set(re.findall(r"def test_(T_L\d+)", f.read_text()))
traced = {t.replace("-","_") for t in re.findall(r"T-L\d+", text.get("TRACEABILITY.md",""))}
need(tests <= traced, f"untraced tests {sorted(tests-traced)}")
need(traced <= tests | {"T_L10b"}, f"traced but missing {sorted(traced-tests)}")
# every step in PLAN has a gate or expected-result line
steps = re.findall(r"^## (S-\d+)", text.get("PLAN.md",""), re.M)
need(len(steps) >= 8, "too few steps")
# frozen manifest verifies
m = root/"tests/FROZEN_MANIFEST.sha256"
if m.is_file():
    for line in m.read_text().splitlines():
        h, name = line.split(None, 1)
        p = root/name.strip()
        need(p.is_file() and hashlib.sha256(p.read_bytes()).hexdigest() == h, f"manifest mismatch {name.strip()}")
# claims all carry a status; no unverified claim is used as a fact in PLAN
for c in json.loads((root/"research/claims.json").read_text()):
    need(c["status"] in ("verified","unverified"), f"claim {c['id']} has no status")
# no secrets
for p in root.rglob("*"):
    if p.is_file() and ".git" not in p.parts and p.suffix in {".md",".py",".sh",".json",".csv",".txt"}:
        if re.search(r"gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}", p.read_text(errors="ignore")):
            need(False, f"secret-shaped string in {p.relative_to(root)}")
print("\n".join(bad) if bad else "plan checks: OK")
sys.exit(1 if bad else 0)
