# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L01 vendored pair tools unchanged (D-981 hashes); T-L09 scorer arithmetic. Serves SC-3, SC-6."""
import base64, hashlib, json, subprocess, sys
from helpers import SHA_MAKE, SHA_SCORE

def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()

def test_T_L01_vendored_tools_match_recorded_hashes(repo):
    """T-L01 (SC-3): scripts/make_blind_pairs.py and scripts/score_blind.py are byte-identical to the D-981 record."""
    assert sha(repo / "scripts" / "make_blind_pairs.py") == SHA_MAKE
    assert sha(repo / "scripts" / "score_blind.py") == SHA_SCORE

def test_T_L09_scorer_p_values(repo, tmp_path):
    """T-L09 (SC-6): exact one-sided binomial p for 12 pairs: 10 -> 0.0193, 9 -> 0.0730, 8 -> 0.1938, 7 -> 0.3872."""
    key = {f"pair-{i:02d}": "A" for i in range(1, 13)}
    k = tmp_path / "key.json.b64"
    k.write_text(base64.b64encode(json.dumps(key).encode()).decode() + "\n")
    for correct, expected in [(12, "0.0002"), (10, "0.0193"), (9, "0.0730"), (8, "0.1938"), (7, "0.3872"), (6, "0.6128")]:
        answers = ",".join(["A"] * correct + ["B"] * (12 - correct))
        out = subprocess.run([sys.executable, str(repo / "scripts" / "score_blind.py"), "--key", str(k), "--answers", answers],
                             capture_output=True, text=True)
        assert out.returncode == 0, out.stderr
        assert f"{correct}/12 correct" in out.stdout and f"p = {expected}" in out.stdout, out.stdout
