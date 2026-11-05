# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L06 event renderer reproduces the verified metrics-mode clips; T-L07 determinism; T-L13 controls act. Serves SC-2."""
import pathlib, subprocess
import numpy as np
import soundfile as sf

HEAD = "stem,time_s,instrument,velocity,hammer,strike,damping,room,pitch_hz,seed,duration_s\n"

def build(repo, tmp_path):
    out = {}
    for name, src, extra in [("render_demo", "Tools/render_demo.cpp", []), ("render_events", "Tools/listening/render_events.cpp", [])]:
        exe = tmp_path / name
        p = subprocess.run(["g++", "-O2", "-std=c++17", "-I", str(repo / "Tools"), str(repo / src), "-o", str(exe), *extra],
                           capture_output=True, text=True, cwd=repo)
        assert p.returncode == 0, p.stderr[-2000:]
        out[name] = exe
    return out

def clips(exe, csv, outdir):
    p = subprocess.run([str(exe), "--clips", str(csv), str(outdir)], capture_output=True, text=True)
    assert p.returncode == 0, p.stdout + p.stderr
    return outdir

def test_T_L06_defaults_reproduce_metrics_mode_bit_for_bit(repo, tmp_path):
    """T-L06 (SC-2): a row with defaults (wood, strike 0.5, damping 1, room 0, no pitch, no seed) equals render_demo metrics i03_v70.wav."""
    exe = build(repo, tmp_path)
    m = tmp_path / "m"; m.mkdir()
    assert subprocess.run([str(exe["render_demo"]), "metrics", str(m)], capture_output=True).returncode == 0
    csv = tmp_path / "e.csv"
    csv.write_text(HEAD + "i03_v70,0,3,0.70,wood,0.5,1.0,0.0,,,3.0\n")
    o = clips(exe["render_events"], csv, tmp_path / "o")
    a, fa = sf.read(m / "i03_v70.wav", dtype="float32"); b, fb = sf.read(o / "i03_v70.wav", dtype="float32")
    assert fa == fb == 48000 and a.shape == b.shape and np.array_equal(a, b)

def test_T_L07_same_row_same_bytes(repo, tmp_path):
    """T-L07 (SC-2): rendering the same file twice gives identical bytes; changing seed changes the noise-driven instruments."""
    exe = build(repo, tmp_path)
    csv = tmp_path / "e.csv"
    csv.write_text(HEAD + "a,0,11,0.9,wood,0.5,1.0,0.0,,7,2.0\n")
    o1 = clips(exe["render_events"], csv, tmp_path / "o1"); o2 = clips(exe["render_events"], csv, tmp_path / "o2")
    assert (o1 / "a.wav").read_bytes() == (o2 / "a.wav").read_bytes()
    csv.write_text(HEAD + "a,0,11,0.9,wood,0.5,1.0,0.0,,8,2.0\n")
    o3 = clips(exe["render_events"], csv, tmp_path / "o3")
    assert (o1 / "a.wav").read_bytes() != (o3 / "a.wav").read_bytes()

def test_T_L13_controls_change_the_sound_and_pitch_is_clamped_visibly(repo, tmp_path):
    """T-L13 (SC-2): hammer, strike, damping, room and pitch each change the render; pitch outside the instrument range is
    clamped and reported on stderr as 'pitch_clamped'."""
    exe = build(repo, tmp_path)
    base = "t,0,5,0.8,wood,0.5,1.0,0.0,,1,2.0"
    variants = {"hammer": "t,0,5,0.8,metal,0.5,1.0,0.0,,1,2.0", "strike": "t,0,5,0.8,wood,0.9,1.0,0.0,,1,2.0",
                "damping": "t,0,5,0.8,wood,0.5,2.0,0.0,,1,2.0", "room": "t,0,5,0.8,wood,0.5,1.0,0.3,,1,2.0",
                "pitch": "t,0,5,0.8,wood,0.5,1.0,0.0,150,1,2.0"}
    ref = None
    for name, row in {"base": base, **variants}.items():
        csv = tmp_path / f"{name}.csv"; csv.write_text(HEAD + row + "\n")
        o = clips(exe["render_events"], csv, tmp_path / name)
        data = (o / "t.wav").read_bytes()
        if name == "base": ref = data
        else: assert data != ref, f"{name} had no effect"
    csv = tmp_path / "clamp.csv"; csv.write_text(HEAD + "t,0,0,1.0,wood,0.5,1.0,0.0,5000,1,1.0\n")
    p = subprocess.run([str(exe["render_events"]), "--clips", str(csv), str(tmp_path / "c")], capture_output=True, text=True)
    assert p.returncode == 0 and "pitch_clamped" in p.stderr
