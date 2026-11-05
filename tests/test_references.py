# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L08 reference manifest and ingest. Serves SC-1."""
import hashlib, json
import soundfile as sf
from helpers import hit, tool

OK_LICENCES = {"CC0-1.0", "CC-BY-4.0", "CC-BY-3.0", "owner-attested-CC0-1.0", "owner-attested-CC-BY-4.0"}

def make(tmp_path, n=12, licence="CC0-1.0", bad_sha=False, family_cycle=("bass", "snare", "tom", "cymbal")):
    src = tmp_path / "src"; src.mkdir()
    rows = []
    for i in range(n):
        f = src / f"take{i:02d}.wav"
        sf.write(f, hit(44100, 2.0, 100 + 30 * i, 5.0, seed=i), 44100, subtype="PCM_24")
        sha = hashlib.sha256(f.read_bytes()).hexdigest()
        rows.append({"stem": f"s{i:02d}", "file": f.name, "sha256": ("0" * 64 if bad_sha and i == 0 else sha),
                     "instrument": i % 15, "family": family_cycle[i % len(family_cycle)], "velocity": 1.0, "hammer": "wood",
                     "licence": licence, "source": "https://example.invalid/take" if not licence.startswith("owner") else "owner",
                     "attestation": "ATTESTATION.md" if licence.startswith("owner") else ""})
    m = tmp_path / "MANIFEST.json"
    m.write_text(json.dumps({"schema_version": 1, "references": rows}))
    return m, src

def ingest(repo, m, src, out):
    return tool(repo, "ingest_refs.py", "--manifest", m, "--src", src, "--out", out)

def test_T_L08_accepts_a_complete_manifest(repo, tmp_path):
    """T-L08a: 12 hashed, licensed, family-covered files are canonicalised to 24-bit mono 44.1 kHz <stem>.wav."""
    m, src = make(tmp_path)
    p = ingest(repo, m, src, tmp_path / "ref")
    assert p.returncode == 0, p.stdout + p.stderr
    assert sorted(x.name for x in (tmp_path / "ref").glob("*.wav")) == [f"s{i:02d}.wav" for i in range(12)]
    assert sf.info(tmp_path / "ref" / "s00.wav").subtype == "PCM_24"

def test_T_L08_rejects_incomplete_or_unlicensed(repo, tmp_path):
    """T-L08b: fewer than 12, a checksum mismatch, a licence outside the allowed set, or fewer than 3 families all fail."""
    good = tmp_path / "good"; good.mkdir()
    m0, s0 = make(good)
    assert ingest(repo, m0, s0, good / "ref").returncode == 0, "a valid manifest must be accepted before rejections mean anything"
    cases = ({"n": 11}, {"bad_sha": True}, {"licence": "CC-BY-NC-4.0"}, {"family_cycle": ("bass", "snare")})
    for idx, kw in enumerate(cases):
        d = tmp_path / f"case{idx}"; d.mkdir()
        m, src = make(d, **kw)
        assert ingest(repo, m, src, d / "ref").returncode != 0, f"accepted {kw}"

def test_T_L08_owner_attested_needs_attestation_file(repo, tmp_path):
    """T-L08c: an owner-attested licence requires the named attestation file next to the manifest."""
    m, src = make(tmp_path, licence="owner-attested-CC0-1.0")
    assert ingest(repo, m, src, tmp_path / "ref").returncode != 0
    (tmp_path / "ATTESTATION.md").write_text("I made these recordings and release them under CC0-1.0.\n")
    assert ingest(repo, m, src, tmp_path / "ref2").returncode == 0

def test_T_L14_make_events_applies_the_pitch_rule(repo, tmp_path):
    """T-L14 (SC-2): event rows use the owner pitch when present, else the f0 estimate; out-of-range pitches are reported and can be dropped."""
    m, src = make(tmp_path)
    data = json.loads(m.read_text())
    data["references"][0]["pitch_hz"] = 64.0            # instrument 0 (bass) range is 30-120 Hz: in range
    data["references"][1]["pitch_hz"] = 5000.0          # instrument 1 (snare 1) range is 100-400 Hz: out of range
    m.write_text(json.dumps(data))
    voicing = repo / "Tools" / "voicing.json"
    inst = json.loads(voicing.read_text())["instruments"]
    mid = lambda i: round((inst[i]["minHz"] * inst[i]["maxHz"]) ** 0.5, 1)       # in range for every instrument
    f0 = {r["stem"]: mid(r["instrument"]) for r in data["references"]}
    (tmp_path / "f0.json").write_text(json.dumps(f0))
    out = tmp_path / "e.csv"
    base = ["--manifest", m, "--f0", tmp_path / "f0.json", "--voicing", voicing, "--out", out]
    p = tool(repo, "make_events.py", *base)
    assert p.returncode == 0, p.stdout + p.stderr
    rows = out.read_text().splitlines()
    assert rows[0] == "stem,time_s,instrument,velocity,hammer,strike,damping,room,pitch_hz,seed,duration_s" and len(rows) == 13
    assert rows[1].split(",")[8] == "64.0" and rows[2].split(",")[8] == "5000.0" and rows[3].split(",")[8] == str(f0["s02"])
    assert "pitch_clamped s01 5000.0 400.0" in p.stderr and p.stderr.count("pitch_clamped") == 1
    q = tool(repo, "make_events.py", *base, "--drop-clamped")
    assert q.returncode == 0 and len(out.read_text().splitlines()) == 12
