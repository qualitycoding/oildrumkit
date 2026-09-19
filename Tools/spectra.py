#!/usr/bin/env python3
"""spectra.py -- spectrum / spectrogram / per-octave RT60 plot for a WAV window.

  spectra.py in.wav out.png [--t0 S] [--t1 S] [--title T]
  spectra.py --batch METRICS_DIR OUT_DIR [--tag before]     # all 15 instruments, v=1.0 hits
"""
import argparse, os, sys
import numpy as np
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
sys.path.insert(0, os.path.dirname(__file__))
import analysis as A


def plot_one(x, sr, out, t0=0.0, t1=None, title=""):
    t1 = t1 if t1 is not None else len(x) / sr
    seg = x[int(t0 * sr):int(t1 * sr)]
    fig, ax = plt.subplots(1, 3, figsize=(17, 4.6))
    f, db = A.spectrum_db(seg, sr, 0.0, None)
    ax[0].semilogx(f[1:], db[1:], lw=0.6); ax[0].set_xlim(30, 20000); ax[0].set_ylim(-100, 3)
    ax[0].set_xlabel("Hz"); ax[0].set_ylabel("dB re peak"); ax[0].set_title("Magnitude spectrum (log-log)"); ax[0].grid(alpha=.3, which="both")
    nper = 1024
    ax[1].specgram(seg, NFFT=nper, Fs=sr, noverlap=nper * 3 // 4, cmap="magma", vmin=-140, vmax=-20)
    ax[1].set_ylim(0, 16000); ax[1].set_xlabel("s"); ax[1].set_ylabel("Hz"); ax[1].set_title("Spectrogram 0-16 kHz")
    tab = A.band_t60_table(seg, sr)
    xs = [str(k) for k in tab]; ys = [v if v is not None else 0 for v in tab.values()]
    ax[2].bar(xs, ys, color=["#b5602c" if v else "#888" for v in tab.values()])
    ax[2].set_ylabel("RT60 (s)"); ax[2].set_xlabel("octave band (Hz)"); ax[2].set_title("Per-octave RT60 (grey = no energy)")
    fig.suptitle(title); fig.tight_layout(); fig.savefig(out, dpi=90); plt.close(fig)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("inp", nargs="?"); ap.add_argument("out", nargs="?")
    ap.add_argument("--t0", type=float, default=0.0); ap.add_argument("--t1", type=float, default=None)
    ap.add_argument("--title", default="")
    ap.add_argument("--batch", nargs=2, metavar=("METRICS_DIR", "OUT_DIR"))
    ap.add_argument("--tag", default="")
    a = ap.parse_args()
    if a.batch:
        d, o = a.batch; os.makedirs(o, exist_ok=True)
        for i, name in enumerate(A.INSTRUMENTS):
            sr, x = A.load(os.path.join(d, f"i{i:02d}_v100.wav"))
            plot_one(A.mono(x), sr, os.path.join(o, f"spectra_{i:02d}_{name.replace(' ', '').replace('-', '')}.png"),
                     0.0, 2.9, f"{a.tag} - {name} (v=1.0, single hit, dry)")
        print("wrote", len(A.INSTRUMENTS), "PNGs to", o)
    else:
        sr, x = A.load(a.inp)
        plot_one(A.mono(x), sr, a.out, a.t0, a.t1, a.title or os.path.basename(a.inp))


if __name__ == "__main__":
    main()
