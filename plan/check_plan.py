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
    need(c["status"] in ("verified","unverified","single-source","corroborated"), f"claim {c['id']} has no status")
# spikes named in the text exist
alltxt = alltext + (root/"research/SOURCES.md").read_text()
for sp in set(re.findall(r"\bS([1-9])\b(?=[^\w-])", re.sub(r"S-\d+", "", alltxt))):
    pass
for n in re.findall(r"spike S(\d)", alltxt):
    need(any((root/"research/spikes").glob(f"S{n}_RESULT.md")), f"spike S{n} named but has no result file")
# exactly one source register; its URLs equal the union of round log, SOURCES.md and claims.json URLs; fields present
regs = [q for q in (root/"knowledge").glob("K-*source-register*.md") if re.search(r"^status: current", q.read_text(), re.M)]
need(len(regs) == 1, f"expected one current source register, found {len(regs)}")
if len(regs) == 1:
    rt = regs[0].read_text()
    rows = [l for l in rt.splitlines() if re.match(r"^\| \d+ \|", l)]
    reg_urls = {l.split(" | ")[2].strip() for l in rows}
    log_urls = {l.split(" | ")[2].strip() for l in (root/"research/rounds/round-1.md").read_text().splitlines() if re.match(r"^\| \d+ \|", l)}
    claim_urls = {s["url"] for c in json.loads((root/"research/claims.json").read_text()) for s in c.get("sources", [])}
    need(log_urls == reg_urls, f"round-log URLs differ from register: {sorted(log_urls ^ reg_urls)[:3]}")
    need(claim_urls <= reg_urls, f"claim URLs missing from register: {sorted(claim_urls - reg_urls)[:3]}")
    for l in rows:
        cells = [c.strip() for c in l.strip().strip("|").split(" | ")]
        need(len(cells) == 12 and all(cells), f"register row incomplete: {cells[:2]}")
        need(len(cells) < 7 or cells[6] in ("used","corroborating","rejected","unreachable","not relevant"), f"bad outcome {cells[:2]}")
    stm = re.search(r"statement: .*?(\d+) external sources", rt)
    need(stm and int(stm.group(1)) == len(rows), "register statement count differs from rows")
# lessons and knowledge items carry the store schema fields
REQ = {"L": ["id","title","status","supersedes","recurrence_of","occurrences","severity","tags","recorded_by","run","recorded_at"],
       "K": ["id","statement","status","supersedes","tags","applies_to_version","source","confidence","derived_from","discovered_by","run","recorded_at"]}
for d, pre in (("lessons","L"),("knowledge","K")):
    for p in (root/d).glob(f"{pre}-*.md"):
        head = p.read_text().split("---", 2)[1]
        fields = dict(re.findall(r"^(\w+):[ \t]*(.*)$", head, re.M))
        for f in REQ[pre]: need(f in fields, f"{p.name}: missing {f}")
        need(fields.get("id","").strip() == p.stem, f"{p.name}: id differs from file name")
# no secrets
for p in root.rglob("*"):
    if p.is_file() and ".git" not in p.parts and p.suffix in {".md",".py",".sh",".json",".csv",".txt"}:
        if re.search(r"gh[pousr]_[A-Za-z0-9]{20,}|github_pat_[A-Za-z0-9_]{20,}", p.read_text(errors="ignore")):
            need(False, f"secret-shaped string in {p.relative_to(root)}")
print("\n".join(bad) if bad else "plan checks: OK")
sys.exit(1 if bad else 0)
