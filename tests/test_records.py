# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L10 round record; T-L11 honest verification (Rule 14). Serves SC-6, SC-7."""
import json, re, pathlib
from helpers import tool

REQUIRED = ["schema_version", "repository", "branch", "candidate_commit", "gate_id", "round", "test_type", "status",
            "prerequisites", "frozen_metrics", "reference", "candidate_renders", "pair_builder", "evidence", "answers",
            "score", "qualitative_findings", "iterations", "response", "resume"]

def test_T_L10_record_round_writes_a_valid_result_and_no_secrets(repo, tmp_path):
    """T-L10 (SC-6, SC-7): record_round.py writes result.json with every key of the common schema and refuses secrets."""
    out = tmp_path / "r1"; out.mkdir()
    p = tool(repo, "record_round.py", "--init", "--round", 1, "--out", out, "--candidate-commit", "0" * 40, "--branch", "impl/listening-validation")
    assert p.returncode == 0, p.stdout + p.stderr
    d = json.loads((out / "result.json").read_text())
    assert all(k in d for k in REQUIRED), [k for k in REQUIRED if k not in d]
    assert d["gate_id"] == "G-LISTEN" and d["round"] == 1 and d["pair_builder"]["seed"] == 20261103
    assert d["pair_builder"]["window"] == "start" and d["pair_builder"]["pairs"] == 12
    assert d["score"]["pass_condition"] == "correct <= 8 && no unresolved qualitative finding"
    (out / "findings.md").write_text("token github_pat_" + "A" * 30 + "\n")
    q = tool(repo, "record_round.py", "--check", "--out", out)
    assert q.returncode != 0, "secret scan did not fail"

def test_T_L11_no_failure_swallowing_in_listening_tools(repo):
    """T-L11 (Rule 14): no `|| true`, `continue-on-error`, `set +e` or bare `except:` in Tools/listening, scripts or workflows."""
    bad = re.compile(r"\|\|\s*true|continue-on-error|set \+e|except\s*:\s*(pass)?\s*$|except Exception:\s*pass")
    hits = []
    roots = [repo / "Tools" / "listening", repo / "scripts", repo / ".github" / "workflows"]
    for r in roots:
        for f in (r.rglob("*") if r.exists() else []):
            if f.is_file() and f.suffix in {".py", ".sh", ".yml", ".yaml", ".cpp"}:
                for n, line in enumerate(f.read_text().splitlines(), 1):
                    if bad.search(line):
                        hits.append(f"{f.relative_to(repo)}:{n}: {line.strip()}")
    assert not hits, "\n".join(hits)
    for name in ("level_plan.py", "verify_bundle.py", "build_round.py", "ingest_refs.py", "record_round.py", "make_events.py", "render_events.cpp"):
        f = repo / "Tools" / "listening" / name
        assert f.is_file() and "NOT IMPLEMENTED" not in f.read_text(), f"{name} missing or still a stub"

def test_T_L10b_score_updates_the_record(repo, tmp_path):
    """T-L10b (SC-6): record_round.py --score runs the vendored scorer and stores correct, total, exact p, same-side count."""
    import base64
    out = tmp_path / "r1"; out.mkdir()
    assert tool(repo, "record_round.py", "--init", "--round", 1, "--out", out, "--candidate-commit", "1" * 40, "--branch", "b").returncode == 0
    key = {f"pair-{i:02d}": ("A" if i % 2 else "B") for i in range(1, 13)}
    k = tmp_path / "key.json.b64"; k.write_text(base64.b64encode(json.dumps(key).encode()).decode() + "\n")
    answers = "A,B,A,B,A,B,A,B,A,A,A,A"      # pairs 1-8 correct, 9 correct, 10 wrong (B expected), 11 correct, 12 wrong
    p = tool(repo, "record_round.py", "--score", "--out", out, "--key", k, "--answers", answers)
    assert p.returncode == 0, p.stdout + p.stderr
    d = json.loads((out / "result.json").read_text())
    assert d["score"]["correct"] == 10 and d["score"]["total"] == 12 and abs(d["score"]["one_sided_p"] - 0.0193) < 1e-4
    assert d["answers"]["ordered"] == answers and d["answers"]["same_side_count"] == 8 and d["status"] == "scored"
    assert (out / "score.txt").read_text().count("pair-") == 12
