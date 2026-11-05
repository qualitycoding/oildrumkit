#!/usr/bin/env python3
"""Verify a built round. Exits 1 and prints every failure.
  verify_bundle.py --round ROUND_DIR --ref REF_DIR
Checks: file set and names; PCM-24 mono 44.1 kHz 1.0 s; no clipping; equal levels (<0.2 dB spread); no stem name or key text in any
listener file; SHA256SUMS; every reference-side clip correlates >= 0.99 with the independently decoded source."""
import argparse, base64, hashlib, pathlib, sys
import numpy as np
from scipy.signal import correlate, resample_poly
sys.path.insert(0, str(pathlib.Path(__file__).parent))
from _common import CORR_MIN, archive_json, read_pcm

ALLOWED = {"README-LISTENER.txt", "ANSWERS.txt", "SHA256SUMS", "ATTRIBUTION.md"}

def norm_corr(a, b):
    n = min(len(a), len(b)); a = a[:n] - a[:n].mean(); b = b[:n] - b[:n].mean()
    return float(correlate(a, b, mode="full", method="fft").max() / (np.linalg.norm(a) * np.linalg.norm(b) + 1e-30))

def source_window(path, fs_out=44100, seconds=1.0):
    x, fs, _, _ = read_pcm(path)
    if fs != fs_out:
        x = resample_poly(x, fs_out, fs)
    i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0])
    return x[i0:i0 + int(fs_out * seconds)]

def verify(round_dir, ref_dir):
    rd, lb = pathlib.Path(round_dir), pathlib.Path(round_dir) / "listener-bundle"
    bad = []
    key, src = archive_json(rd, "key.json.b64"), archive_json(rd, "sources.json.b64")
    names = sorted(p.name for p in lb.glob("pair-*.wav"))
    want = [f"pair-{i:02d}-{s}.wav" for i in range(1, len(key) + 1) for s in "AB"]
    if names != want: bad.append(f"pair files {names} != {want}")
    extra = {p.name for p in lb.iterdir() if not p.name.startswith("pair-")} - ALLOWED - {"demos", "DEMOS.txt"}
    if extra: bad.append(f"unexpected listener files: {sorted(extra)}")
    levels = []
    for n in names:
        x, fs, sw, ch = read_pcm(lb / n)
        if (fs, sw, ch, len(x)) != (44100, 3, 1, 44100): bad.append(f"{n}: format {(fs, sw, ch, len(x))}")
        if np.max(np.abs(x)) >= 0.999: bad.append(f"{n}: clipped")
        levels.append(20 * np.log10(np.sqrt(np.mean((x - x.mean()) ** 2)) + 1e-12))
    if levels and max(levels) - min(levels) > 0.2: bad.append(f"level spread {max(levels) - min(levels):.2f} dB")
    blob = b"".join(p.read_bytes() for p in lb.rglob("*") if p.is_file())
    for stem in set(src.values()):
        if stem.encode() in blob: bad.append(f"stem name {stem} appears in the listener bundle")
    if (rd / "archive" / "key.json.b64").read_text().strip().encode() in blob: bad.append("key text appears in the listener bundle")
    sums = lb / "SHA256SUMS"
    if sums.is_file():
        for line in sums.read_text().splitlines():
            h, _, f = line.partition("  ")
            if hashlib.sha256((lb / f).read_bytes()).hexdigest() != h: bad.append(f"SHA256SUMS mismatch for {f}")
    else: bad.append("SHA256SUMS missing")
    for pair, side in sorted(key.items()):
        f = lb / f"{pair}-{side}.wav"
        if not f.is_file(): continue
        c = norm_corr(read_pcm(f)[0], source_window(pathlib.Path(ref_dir) / f"{src[pair]}.wav"))
        print(f"{pair} reference side {side}: correlation {c:.5f}")
        if c < CORR_MIN: bad.append(f"{pair}: reference-side correlation {c:.3f} < {CORR_MIN}")
    return bad

def main(argv=None):
    ap = argparse.ArgumentParser(); ap.add_argument("--round", required=True); ap.add_argument("--ref", required=True)
    a = ap.parse_args(argv)
    bad = verify(a.round, a.ref)
    for b in bad: print("FAIL:", b)
    print("verify_bundle:", "FAILED" if bad else "ok")
    return 1 if bad else 0

if __name__ == "__main__":
    sys.exit(main())
