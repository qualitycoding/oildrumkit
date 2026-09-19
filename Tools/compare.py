#!/usr/bin/env python3
"""compare.py -- before/after comparison.

  compare.py BEFORE_HITS AFTER_HITS BEFORE.json AFTER.json OUT_DIR [--ref REAL.wav]

Writes OUT_DIR/compare_XX_<name>.png (overlaid spectra, side-by-side spectrograms, RT60 bars) and
OUT_DIR/comparison.md (metric table + the Phase-5 numeric targets, each marked PASS/FAIL honestly).
--ref adds a third column from a real recording (never available in the sandbox used for development).
"""
import argparse, json, os, sys
import numpy as np
import matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
sys.path.insert(0, os.path.dirname(__file__))
import analysis as A


def panel(ax_spec, ax_sg, x, sr, label, color, ymax=16000):
    f, db = A.spectrum_db(x, sr, 0.0, 2.9)
    ax_spec.semilogx(f[1:], db[1:], lw=0.5, color=color, label=label, alpha=0.85)
    ax_sg.specgram(x[: int(2.9 * sr)], NFFT=1024, Fs=sr, noverlap=768, cmap="magma", vmin=-140, vmax=-20)
    ax_sg.set_ylim(0, ymax); ax_sg.set_title(label, fontsize=9)


def main():
    ap = argparse.ArgumentParser()
    for a in ("before_hits", "after_hits", "before_json", "after_json", "out"): ap.add_argument(a)
    ap.add_argument("--ref", default=None)
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    B = json.load(open(a.before_json)); Af = json.load(open(a.after_json))
    ref = None
    if a.ref:
        rsr, rx = A.load(a.ref); ref = (rsr, A.mono(rx))

    for i, name in enumerate(A.INSTRUMENTS):
        sb, xb = A.load(os.path.join(a.before_hits, f"i{i:02d}_v100.wav"))
        sa, xa = A.load(os.path.join(a.after_hits, f"i{i:02d}_v100.wav"))
        xb, xa = A.mono(xb), A.mono(xa)
        cols = 3 + (1 if ref else 0)
        fig = plt.figure(figsize=(5 * cols, 4.4))
        ax0 = fig.add_subplot(1, cols, 1)
        panel(ax0, plt.figure().add_subplot(111), xb, sb, "before", "#888")   # scratch axes for spectrogram
        plt.close(plt.gcf())
        ax0.cla(); ax1 = fig.add_subplot(1, cols, 2); ax2 = fig.add_subplot(1, cols, 3)
        fb, dbb = A.spectrum_db(xb, sb, 0, 2.9); fa, dba = A.spectrum_db(xa, sa, 0, 2.9)
        # common normalisation: both re the 'after' peak scaled by their own RMS so shapes are comparable
        ax0.semilogx(fb[1:], dbb[1:], lw=0.5, color="#888", label="before")
        ax0.semilogx(fa[1:], dba[1:], lw=0.5, color="#b5602c", label="after", alpha=0.85)
        ax0.set_xlim(30, 20000); ax0.set_ylim(-100, 3); ax0.grid(alpha=.3, which="both"); ax0.legend(fontsize=8)
        ax0.set_title("spectra (each re its own peak)", fontsize=9)
        for ax, x, sr, lab in ((ax1, xb, sb, "before"), (ax2, xa, sa, "after")):
            ax.specgram(x[: int(2.9 * sr)], NFFT=1024, Fs=sr, noverlap=768, cmap="magma", vmin=-140, vmax=-20)
            ax.set_ylim(0, 16000); ax.set_title(f"{lab} spectrogram", fontsize=9)
        if ref:
            ax3 = fig.add_subplot(1, cols, 4)
            ax3.specgram(ref[1][: int(2.9 * ref[0])], NFFT=1024, Fs=ref[0], noverlap=768, cmap="magma", vmin=-140, vmax=-20)
            ax3.set_ylim(0, 16000); ax3.set_title("real recording", fontsize=9)
        fig.suptitle(name); fig.tight_layout()
        fig.savefig(os.path.join(a.out, f"compare_{i:02d}_{name.replace(' ', '').replace('-', '')}.png"), dpi=80); plt.close("all")

    # ---- table + targets
    L = ["# Before / after metrics", "", "Single hits at v=1.0, dry, 48 kHz. `*` = window fell back to t=10 ms (signal < -50 dB by 50 ms).", "",
         "| instrument | partials @50 ms (before -> after) | HF>2k energy per +velocity (dB, after) | T60 HF/LF (before -> after) | peak dBFS (before -> after) |",
         "|---|---|---|---|---|"]
    fr = lambda v: f"{v:.2f}" if v is not None else "-"
    for name in A.INSTRUMENTS:
        b, f = B["instruments"][name], Af["instruments"][name]
        vt = Af.get("velocity_timbre", {}).get(name, {})
        L.append(f"| {name} | {b['partials_at_50ms']} -> {f['partials_at_50ms']}{'*' if f['partials_fallback_10ms'] else ''} "
                 f"| {vt.get('hf2k_fraction_delta_db', float('nan')):+.2f} "
                 f"| {fr(b['t60_hf_over_lf'])} -> {fr(f['t60_hf_over_lf'])} | {b['peak_dbfs']:.1f} -> {f['peak_dbfs']:.1f} |")
    rb, ra = B["snare_roll_xcorr"], Af["snare_roll_xcorr"]
    L += ["", "## Snare roll repeatability (normalised cross-correlation of consecutive hits; lower = less machine-gun)", "",
          "| band | before mean/max | after mean/max |", "|---|---|---|",
          f"| full band | {rb['full_mean']:.3f} / {rb['full_max']:.3f} | {ra['full_mean']:.3f} / {ra['full_max']:.3f} |",
          f"| 100-1000 Hz | {rb['mid100_1k_mean']:.3f} / {rb['mid100_1k_max']:.3f} | {ra['mid100_1k_mean']:.3f} / {ra['mid100_1k_max']:.3f} |",
          f"| > 1 kHz | {rb['hp1k_mean']:.3f} / {rb['hp1k_max']:.3f} | {ra['hp1k_mean']:.3f} / {ra['hp1k_max']:.3f} |"]
    # targets
    n_ok = sum(1 for n in A.INSTRUMENTS if Af["instruments"][n]["partials_at_50ms"] >= 25)
    cen_ok = sum(1 for n in A.INSTRUMENTS if Af.get("velocity_timbre", {}).get(n, {}).get("centroid_delta_avg_hz", 0) >= 400)
    ratios = [Af["instruments"][n]["t60_hf_over_lf"] for n in A.INSTRUMENTS if Af["instruments"][n]["t60_hf_over_lf"] is not None]
    hf_ok = sum(1 for r in ratios if r < 0.25)
    L += ["", "## Phase 5 numeric targets (as written in the plan) -- reported honestly", "",
          f"1. Resolved partials >= 25 at 50 ms: **{n_ok}/15 instruments meet it** ({'PASS' if n_ok == 15 else 'FAIL'}).",
          f"2. Centroid rise v0.2 -> v1.0 >= 400 Hz: **{cen_ok}/15 meet it** ({'PASS' if cen_ok == 15 else 'FAIL'}); before was exactly 0 for all.",
          f"3. Successive snare-roll hits, full-band xcorr <= 0.97: max {ra['full_max']:.3f} ({'PASS' if ra['full_max'] <= 0.97 else 'FAIL'}); "
          f"NOTE the legacy engine also passed this (max {rb['full_max']:.3f}), so it does not discriminate -- see the 100-1000 Hz row.",
          f"4. HF (4-8 kHz) T60 < 25 % of LF T60: **{hf_ok}/{len(ratios)} measurable instruments meet it** ({'PASS' if hf_ok == len(ratios) else 'FAIL'})."]
    open(os.path.join(a.out, "comparison.md"), "w").write("\n".join(L) + "\n")
    print("\n".join(L))


if __name__ == "__main__":
    main()
