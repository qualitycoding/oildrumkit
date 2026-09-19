#!/usr/bin/env python3
"""metrics.py -- objective A/B metrics for a render set.

  metrics.py METRICS_DIR DEMO_PREFIX OUT.json [--label before]

METRICS_DIR: isolated single hits iXX_vYY.wav from `render_demo metrics` (float32, dry)
DEMO_PREFIX: prefix of a demo render (needs PREFIX.wav is NOT used; uses PREFIX_dry.wav if present else PREFIX.wav,
             plus PREFIX.events.csv) -- for the snare-roll repeatability metric.
"""
import argparse, csv, json, os, sys
import numpy as np
sys.path.insert(0, os.path.dirname(__file__))
import analysis as A


def hit(dirn, i, v):
    sr, x = A.load(os.path.join(dirn, f"i{i:02d}_v{v:02d}.wav"))
    return sr, A.mono(x)


def partials_metric(x, sr):
    """Partial count at t=50 ms (200 ms Hann window). Falls back to t=10 ms if the window is
    >50 dB below the first 50 ms, and flags it."""
    def rms(a, b): return np.sqrt(np.mean(x[int(a * sr):int(b * sr)].astype(np.float64) ** 2) + 1e-30)
    if 20 * np.log10(rms(0.05, 0.25) / rms(0.0, 0.05)) < -50:
        return A.count_partials(x, sr, t0=0.01, dur=0.1), True
    return A.count_partials(x, sr, t0=0.05, dur=0.2), False


def lf_hf_t60(x, sr, name_idx):
    """(lf_label, lf_t60, hf_t60, ratio). LF = 80-200 Hz, HF = 4-8 kHz; instruments with no
    energy in 80-200 Hz (hats/cymbals/cowbell) use 500-1000 Hz as LF."""
    hf = A.band_t60(x, sr, 0, edges=(4000, 8000))
    for label, edges in (("80-200", (80, 200)), ("500-1k", (500, 1000))):
        lf = A.band_t60(x, sr, 0, edges=edges)
        if lf is not None:
            break
    else:
        label = "none"
    ratio = (hf / lf) if (lf and hf) else None
    return label, lf, hf, ratio


def velocity_metrics(veldir):
    """Seed-averaged (8 hits) timbre change from v=0.2 to v=1.0, first 100 ms:
       centroid delta [Hz] and change in the fraction of energy above 2 kHz [dB]."""
    out = {}
    for i, name in enumerate(A.INSTRUMENTS):
        cen = {20: [], 100: []}; hf = {20: [], 100: []}
        for v in (20, 100):
            for s in range(8):
                sr, x = A.load(os.path.join(veldir, f"i{i:02d}_v{v:02d}_s{s}.wav")); x = A.mono(x)[: int(0.1 * sr)]
                cen[v].append(A.spectral_centroid(x, sr, 0.0, 0.1))
                P = np.abs(np.fft.rfft(x.astype(np.float64) * np.hanning(len(x)), 1 << 15)) ** 2
                f = np.fft.rfftfreq(1 << 15, 1.0 / sr)
                hf[v].append(P[f >= 2000].sum() / (P.sum() + 1e-30))
        d = float(np.mean(cen[100]) - np.mean(cen[20]))
        db = float(10 * np.log10(np.mean(hf[100]) / (np.mean(hf[20]) + 1e-30) + 1e-30))
        out[name] = dict(centroid_delta_avg_hz=d, hf2k_fraction_delta_db=db)
    return out


def roll_metric(demo_prefix):
    ev = list(csv.DictReader(open(demo_prefix + ".events.csv")))
    p = demo_prefix + "_dry.wav" if os.path.exists(demo_prefix + "_dry.wav") else demo_prefix + ".wav"
    sr, x = A.load(p); x = A.mono(x)
    d = [float(e["time_s"]) for e in ev if e["section"] == "D"]
    seg = int(round((d[1] - d[0]) * sr))
    segs = [x[int(t * sr):int(t * sr) + seg] for t in d[1:-1]]     # skip first (no prior tail) & last
    def bp(a, lo, hi):
        from scipy.signal import butter, sosfilt
        return sosfilt(butter(4, [lo, hi], btype="band", fs=sr, output="sos"), np.asarray(a, dtype=np.float64))
    full = [A.xcorr_peak(segs[k], segs[k + 1]) for k in range(len(segs) - 1)]
    mid = [A.xcorr_peak(bp(segs[k], 100, 1000), bp(segs[k + 1], 100, 1000)) for k in range(len(segs) - 1)]
    hp = [A.xcorr_peak(A.highpass(segs[k], sr, 1000), A.highpass(segs[k + 1], sr, 1000)) for k in range(len(segs) - 1)]
    return dict(full_mean=float(np.mean(full)), full_max=float(np.max(full)),
                mid100_1k_mean=float(np.mean(mid)), mid100_1k_max=float(np.max(mid)),
                hp1k_mean=float(np.mean(hp)), hp1k_max=float(np.max(hp)), n_pairs=len(full))


def main():
    ap = argparse.ArgumentParser(); ap.add_argument("metrics_dir"); ap.add_argument("demo_prefix"); ap.add_argument("out")
    ap.add_argument("--label", default=""); ap.add_argument("--veldir", default=None); a = ap.parse_args()
    out = dict(label=a.label, instruments={})
    for i, name in enumerate(A.INSTRUMENTS):
        sr, x10 = hit(a.metrics_dir, i, 100)
        _, x02 = hit(a.metrics_dir, i, 20)
        n50, fell = partials_metric(x10, sr)
        c02 = A.spectral_centroid(x02, sr, 0.0, 0.1); c10 = A.spectral_centroid(x10, sr, 0.0, 0.1)
        lab, lf, hf, ratio = lf_hf_t60(x10, sr, i)
        out["instruments"][name] = dict(
            peak_dbfs=float(20 * np.log10(np.abs(x10).max() + 1e-12)),
            partials_at_50ms=n50, partials_fallback_10ms=bool(fell),
            centroid_v02_hz=c02, centroid_v10_hz=c10, centroid_delta_hz=c10 - c02,
            t60_lf_band=lab, t60_lf_s=lf, t60_hf_s=hf, t60_hf_over_lf=ratio,
            t60_octaves={str(k): v for k, v in A.band_t60_table(x10, sr).items()})
    out["snare_roll_xcorr"] = roll_metric(a.demo_prefix)
    if a.veldir:
        vm = velocity_metrics(a.veldir); out["velocity_timbre"] = vm
        print("velocity -> timbre (8-seed mean, first 100 ms):  centroid delta Hz | HF(>2k) energy-fraction delta dB")
        for n, m in vm.items(): print(f"  {n:14s} {m['centroid_delta_avg_hz']:8.0f} | {m['hf2k_fraction_delta_db']:6.2f}")
    json.dump(out, open(a.out, "w"), indent=1)
    print("wrote", a.out)
    hdr = f"{'instrument':14s} {'peak dBFS':>9s} {'partials':>8s} {'dCentroid':>9s} {'LF band':>7s} {'LF T60':>7s} {'HF T60':>7s} {'HF/LF':>6s}"
    print(hdr)
    f = lambda v, fmt: (fmt % v) if v is not None else "  -  "
    for n, m in out["instruments"].items():
        print(f"{n:14s} {m['peak_dbfs']:9.1f} {m['partials_at_50ms']:>7d}{'*' if m['partials_fallback_10ms'] else ' '} {m['centroid_delta_hz']:9.0f} "
              f"{m['t60_lf_band']:>7s} {f(m['t60_lf_s'],'%7.2f'):>7s} {f(m['t60_hf_s'],'%7.2f'):>7s} {f(m['t60_hf_over_lf'],'%6.2f'):>6s}")
    r = out["snare_roll_xcorr"]
    print(f"snare roll xcorr (consecutive hits): full-band mean {r['full_mean']:.3f} max {r['full_max']:.3f} | 100-1k mean {r['mid100_1k_mean']:.3f} max {r['mid100_1k_max']:.3f} | >1k mean {r['hp1k_mean']:.3f} max {r['hp1k_max']:.3f}")


if __name__ == "__main__":
    main()
