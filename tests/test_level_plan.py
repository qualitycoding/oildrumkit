# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""T-L05 level planner. Serves SC-2 (the builder aborts on percussive crest factors at -23 dB; spike S2)."""
import math, re
import numpy as np
import soundfile as sf
from helpers import hit, tool, write_set

def crest_db(x44):
    seg = x44 - x44.mean()
    return 20 * math.log10(np.max(np.abs(seg)) / math.sqrt(np.mean(seg ** 2)))

def planned(repo, ref, cand):
    p = tool(repo, "level_plan.py", "--ref", ref, "--cand", cand, "--seconds", 1.0, "--fs", 44100, "--margin-db", 0.5)
    assert p.returncode == 0, p.stdout + p.stderr
    m = re.search(r"^rms_db=(-?\d+)$", p.stdout, re.M)
    assert m, p.stdout
    return int(m.group(1))

def test_T_L05_level_is_cap_when_all_clips_fit(repo, tmp_path):
    """T-L05a: low-crest clips -> rms_db = -23 (never louder than the D-981 level)."""
    ref, cand = tmp_path / "ref", tmp_path / "cand"
    ref.mkdir(); cand.mkdir()
    for d in (ref, cand):
        sf.write(d / "s1.wav", 0.1 * np.sin(2 * np.pi * 220 * np.arange(44100) / 44100), 44100, subtype="PCM_24")
    assert planned(repo, ref, cand) == -23

def test_T_L05_level_follows_highest_crest_factor(repo, tmp_path):
    """T-L05b: with percussive hits the plan is floor(-(max crest) - 0.5) over BOTH sides, computed independently here."""
    ref, cand = tmp_path / "ref", tmp_path / "cand"
    write_set(ref, cand, [f"s{i:02d}" for i in range(12)])
    worst = 0.0
    for d in (ref, cand):
        for f in d.glob("*.wav"):
            x, fs = sf.read(f)
            x = x.mean(axis=1) if x.ndim > 1 else x
            from scipy.signal import resample_poly
            if fs != 44100:
                x = resample_poly(x, 44100, fs)
            i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0])
            seg = x[i0:i0 + 44100]
            seg = np.pad(seg, (0, 44100 - len(seg)))
            worst = max(worst, crest_db(seg))
    expected = min(-23, math.floor(-worst - 0.5))
    assert planned(repo, ref, cand) == expected
    assert expected < -23, "fixture must be percussive enough to need a lower level"
