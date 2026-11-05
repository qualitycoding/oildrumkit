# FROZEN - DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
"""Shared helpers for the listening-phase tests. Independent of the tools under test: it decodes WAV files with the
standard-library `wave` module, not with soundfile, so a decoding bug in the tools cannot hide itself (lesson
L-20261106T010000Z-verify-blind-references-against-sources)."""
import base64, json, pathlib, subprocess, sys, wave
import numpy as np
import soundfile as sf
from scipy.signal import correlate, resample_poly

SEED_BASE = 20261103
SHA_MAKE = "3c4f75854e2a692d3a77460d470e289be405c2166bbc30abbd4eeb053d6824c7"
SHA_SCORE = "f2d9ea5c76a731fd3dd553d698a9f1a92b393d272f6ea898e6a9e5cbde49defa"

def tool(repo, name, *args, check=False):
    p = subprocess.run([sys.executable, str(pathlib.Path(repo) / "Tools" / "listening" / name), *map(str, args)],
                       capture_output=True, text=True)
    if check and p.returncode != 0:
        raise AssertionError(f"{name} exited {p.returncode}\n{p.stdout}\n{p.stderr}")
    return p

def read_pcm(path):
    """Mono float array and sample rate from a 16- or 24-bit PCM WAV, using only the standard library."""
    with wave.open(str(path), "rb") as w:
        n, ch, sw, fs = w.getnframes(), w.getnchannels(), w.getsampwidth(), w.getframerate()
        raw = w.readframes(n)
    if sw == 2:
        x = np.frombuffer(raw, "<i2").astype(float) / 32768.0
    elif sw == 3:
        b = np.frombuffer(raw, np.uint8).reshape(-1, 3)
        v = b[:, 0].astype(np.int32) | (b[:, 1].astype(np.int32) << 8) | (b[:, 2].astype(np.int32) << 16)
        x = np.where(v >= 1 << 23, v - (1 << 24), v) / 8388608.0
    else:
        raise AssertionError(f"unsupported sample width {sw} in {path}")
    return x.reshape(-1, ch).mean(axis=1), fs, sw, ch

def hit(fs=48000, seconds=3.0, f0=180.0, decay=6.0, seed=1, crest_boost=0.0, stereo=False):
    """Synthetic struck-metal hit: a handful of inharmonic damped partials plus a short click."""
    rng = np.random.default_rng(seed)
    t = np.arange(int(fs * seconds)) / fs
    x = np.zeros_like(t)
    for k, r in enumerate([1.0, 2.31, 3.77, 5.12, 6.9, 9.4]):
        x += (0.5 ** k) * np.sin(2 * np.pi * f0 * r * t + rng.uniform(0, 6.28)) * np.exp(-decay * (1 + 0.6 * k) * t)
    x[:40] += crest_boost * rng.standard_normal(40)
    x = 0.4 * x / np.max(np.abs(x))
    return np.stack([x, 0.9 * x], axis=1) if stereo else x

def write_set(refdir, canddir, stems):
    """Reference side: 16-bit mono 44.1 kHz at -12 dB. Candidate side: float32 stereo 48 kHz. Different hits per side."""
    pathlib.Path(refdir).mkdir(parents=True, exist_ok=True)
    pathlib.Path(canddir).mkdir(parents=True, exist_ok=True)
    for i, s in enumerate(stems):
        r = resample_poly(hit(48000, 2.0, 120 + 25 * i, 5.0 + i % 3, seed=i, crest_boost=2.0 * (i % 2)), 441, 480) * 0.25
        sf.write(pathlib.Path(refdir) / f"{s}.wav", r, 44100, subtype="PCM_16")
        sf.write(pathlib.Path(canddir) / f"{s}.wav", hit(48000, 3.0, 125 + 24 * i, 6.0 + i % 4, seed=100 + i, crest_boost=2.0 * ((i + 1) % 2), stereo=True).astype(np.float32), 48000, subtype="FLOAT")

def norm_corr(a, b):
    n = min(len(a), len(b))
    a = a[:n] - a[:n].mean(); b = b[:n] - b[:n].mean()
    c = correlate(a, b, mode="full", method="fft")
    return float(c.max() / (np.linalg.norm(a) * np.linalg.norm(b) + 1e-30))

def source_window(path, fs_out=44100, seconds=1.0):
    x, fs, _, _ = read_pcm(path)
    if fs != fs_out:
        x = resample_poly(x, fs_out, fs)
    i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0])
    return x[i0:i0 + int(fs_out * seconds)]

def load_archive(roundir):
    a = pathlib.Path(roundir) / "archive"
    key = json.loads(base64.b64decode((a / "key.json.b64").read_text()))
    src = json.loads(base64.b64decode((a / "sources.json.b64").read_text()))
    return key, src
