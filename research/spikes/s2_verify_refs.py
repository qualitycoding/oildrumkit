"""Spike for T-L02: ref-side clips must correlate >= 0.99 with an INDEPENDENTLY decoded source (stdlib wave + numpy, no soundfile)."""
import base64, json, sys, wave, pathlib
import numpy as np
from scipy.signal import resample_poly, correlate
import soundfile as sf

def read_pcm(path):
    with wave.open(str(path), "rb") as w:
        n, ch, sw, fs = w.getnframes(), w.getnchannels(), w.getsampwidth(), w.getframerate()
        raw = w.readframes(n)
    if sw == 2: x = np.frombuffer(raw, "<i2").astype(float) / 32768.0
    elif sw == 3:
        b = np.frombuffer(raw, np.uint8).reshape(-1, 3)
        v = (b[:, 0].astype(np.int32) | (b[:, 1].astype(np.int32) << 8) | (b[:, 2].astype(np.int32) << 16))
        v = np.where(v >= 1 << 23, v - (1 << 24), v); x = v / 8388608.0
    else: raise SystemExit(f"unsupported sample width {sw}")
    return x.reshape(-1, ch).mean(axis=1), fs

def norm_corr(a, b):
    n = min(len(a), len(b)); a = a[:n] - a[:n].mean(); b = b[:n] - b[:n].mean()
    c = correlate(a, b, mode="full", method="fft"); 
    return float(c.max() / (np.linalg.norm(a) * np.linalg.norm(b) + 1e-30))

def check(blind, ref_dir, outsr=44100):
    key = json.loads(base64.b64decode((blind / "key.json.b64").read_text()))
    src = json.loads(base64.b64decode((blind / "sources.json.b64").read_text()))
    worst = 1.0
    for pair, side in sorted(key.items()):
        clip, fs = read_pcm(blind / f"{pair}-{side}.wav")
        x, xfs = read_pcm(pathlib.Path(ref_dir) / f"{src[pair]}.wav")
        x = resample_poly(x, outsr, xfs) if xfs != outsr else x
        i0 = int(np.flatnonzero(np.abs(x) > 0.01 * np.abs(x).max())[0]); x = x[i0:i0 + outsr]
        c = norm_corr(clip, x); worst = min(worst, c)
        print(pair, side, src[pair], f"corr={c:.5f}")
    return worst

if __name__ == "__main__":
    w = check(pathlib.Path(sys.argv[1]), sys.argv[2]); print("worst", w); sys.exit(0 if w >= 0.99 else 1)
