# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L02 reference fidelity, T-L03 no leaks, T-L04 pair format, T-L12 seed rule. Serves SC-3, SC-4, SC-5."""
import base64, pathlib, re, shutil
import numpy as np
import soundfile as sf
from helpers import SEED_BASE, hit, load_archive, norm_corr, read_pcm, source_window, tool, write_set

STEMS = [f"stem-{c}" for c in "abcdefghijkl"]
ALLOWED = {"README-LISTENER.txt", "ANSWERS.txt", "SHA256SUMS", "ATTRIBUTION.md"}

def build(repo, tmp_path, rnd=1):
    ref, cand, out = tmp_path / "ref", tmp_path / "cand", tmp_path / f"round-{rnd}"
    write_set(ref, cand, STEMS)
    p = tool(repo, "build_round.py", "--ref", ref, "--cand", cand, "--out", out, "--round", rnd)
    assert p.returncode == 0, p.stdout + p.stderr
    return ref, cand, out

def test_T_L12_seed_rule(repo):
    """T-L12 (SC-5): round seeds are 20261103, +1000, +2000; a fourth round is refused."""
    for rnd, seed in [(1, SEED_BASE), (2, SEED_BASE + 1000), (3, SEED_BASE + 2000)]:
        p = tool(repo, "build_round.py", "--print-seed", "--round", rnd)
        assert p.returncode == 0 and p.stdout.strip() == str(seed), p.stdout + p.stderr
    assert tool(repo, "build_round.py", "--print-seed", "--round", 4).returncode != 0

def test_T_L04_pair_format(repo, tmp_path):
    """T-L04 (SC-3): 12 pairs, 24 mono PCM-24 44.1 kHz 1.0 s clips, equal RMS within 0.1 dB, no clipping, 10 ms fade-out."""
    _, _, out = build(repo, tmp_path)
    lb = out / "listener-bundle"
    names = sorted(p.name for p in lb.glob("pair-*.wav"))
    assert names == [f"pair-{i:02d}-{s}.wav" for i in range(1, 13) for s in "AB"]
    levels = []
    for n in names:
        x, fs, sw, ch = read_pcm(lb / n)
        assert (fs, sw, ch, len(x)) == (44100, 3, 1, 44100), (n, fs, sw, ch, len(x))
        assert np.max(np.abs(x)) < 0.999
        assert abs(x[-1]) < 1e-3, "no fade-out"
        levels.append(20 * np.log10(np.sqrt(np.mean((x - x.mean()) ** 2))))
    assert max(levels) - min(levels) < 0.2, levels

def test_T_L03_listener_bundle_hides_the_key(repo, tmp_path):
    """T-L03 (SC-4): the listener bundle holds only pairs and neutral files; no stem names, key, mapping or side hints."""
    _, _, out = build(repo, tmp_path)
    lb = out / "listener-bundle"
    others = {p.name for p in lb.iterdir() if not p.name.startswith("pair-")}
    assert others <= ALLOWED | {"demos"}, others
    assert (out / "archive" / "key.json.b64").is_file() and (out / "archive" / "sources.json.b64").is_file()
    key_text = (out / "archive" / "key.json.b64").read_text().strip()
    blob = b"".join(p.read_bytes() for p in lb.rglob("*") if p.is_file())
    for stem in STEMS:
        assert stem.encode() not in blob, f"stem name {stem} leaked"
    assert key_text.encode() not in blob
    sheet = (lb / "ANSWERS.txt").read_text()
    assert re.findall(r"^pair-\d\d:[ \t]*$", sheet, re.M) == [f"pair-{i:02d}:" for i in range(1, 13)], sheet
    for word in ("reference was", "candidate", "synth"):
        assert word not in (lb / "README-LISTENER.txt").read_text().lower()

def test_T_L02_reference_side_matches_independent_decode(repo, tmp_path):
    """T-L02 (SC-3): every reference-side clip correlates >= 0.99 with the source decoded by the standard library
    (lesson L-20261106T010000Z). The check also FAILS when a reference-side clip is replaced by noise (Rule 14)."""
    ref, _, out = build(repo, tmp_path)
    key, src = load_archive(out)
    lb = out / "listener-bundle"
    for pair, side in key.items():
        x, fs, _, _ = read_pcm(lb / f"{pair}-{side}.wav")
        c = norm_corr(x, source_window(ref / f"{src[pair]}.wav"))
        assert c >= 0.99, (pair, c)
    p = tool(repo, "verify_bundle.py", "--round", out, "--ref", ref)
    assert p.returncode == 0, p.stdout + p.stderr
    bad = tmp_path / "bad"
    shutil.copytree(out, bad)
    pair, side = sorted(key.items())[3]
    sf.write(bad / "listener-bundle" / f"{pair}-{side}.wav", np.random.default_rng(1).uniform(-0.05, 0.05, 44100), 44100, subtype="PCM_24")
    q = tool(repo, "verify_bundle.py", "--round", bad, "--ref", ref)
    assert q.returncode != 0, "verify_bundle accepted a noise clip as the reference"

def test_T_L16_public_evidence_omits_the_pairs_unless_every_reference_is_redistributable(repo, tmp_path):
    """T-L16 (D-020): the private listener bundle always holds the 24 pair clips. The public-evidence folder holds the pairs only
    when the reference set is marked all_redistributable; a missing marker counts as not redistributable (fail closed). It never
    holds the key or the mapping, and always holds demos-side files, its own SHA256SUMS and the hashes of the private bundle."""
    import json
    ref, cand = tmp_path / "ref", tmp_path / "cand"
    write_set(ref, cand, STEMS)
    closed, opened = tmp_path / "closed", tmp_path / "opened"
    p = tool(repo, "build_round.py", "--ref", ref, "--cand", cand, "--out", closed, "--round", 1)
    assert p.returncode == 0, p.stdout + p.stderr
    (ref / "public_ok.json").write_text(json.dumps({"all_redistributable": True, "stems": STEMS}))
    q = tool(repo, "build_round.py", "--ref", ref, "--cand", cand, "--out", opened, "--round", 1)
    assert q.returncode == 0, q.stdout + q.stderr
    for out, expect in ((closed, 0), (opened, 24)):
        assert len(list((out / "listener-bundle").glob("pair-*.wav"))) == 24
        pub = out / "public-evidence"
        assert len(list(pub.glob("pair-*.wav"))) == expect, (out.name, sorted(x.name for x in pub.iterdir()))
        names = {x.name for x in pub.rglob("*") if x.is_file()}
        assert {"SHA256SUMS", "PRIVATE_BUNDLE.sha256", "README-LISTENER.txt"} <= names
        assert not any("key" in n or "sources" in n or "settings" in n for n in names), names
        blob = b"".join(x.read_bytes() for x in pub.rglob("*") if x.is_file())
        for stem in STEMS:
            assert stem.encode() not in blob
    assert (closed / "public-evidence" / "PRIVATE_BUNDLE.sha256").read_text() == (closed / "listener-bundle" / "SHA256SUMS").read_text()

def test_T_L15_round_is_reproducible_from_committed_inputs(repo, tmp_path):
    """T-L15 (SC-5, R-006): the same inputs and seed rebuild a byte-identical listener bundle and the same key, so a lost
    working copy never forces a new key; a different round number gives a different bundle."""
    ref, cand = tmp_path / "ref", tmp_path / "cand"
    write_set(ref, cand, STEMS)
    outs = []
    for name, rnd in (("a", 1), ("b", 1), ("c", 2)):
        out = tmp_path / name
        p = tool(repo, "build_round.py", "--ref", ref, "--cand", cand, "--out", out, "--round", rnd)
        assert p.returncode == 0, p.stdout + p.stderr
        outs.append(out)
    sums = [(o / "listener-bundle" / "SHA256SUMS").read_text() for o in outs]
    keys = [(o / "archive" / "key.json.b64").read_text() for o in outs]
    assert sums[0] == sums[1] and keys[0] == keys[1]
    assert sums[0] != sums[2]
