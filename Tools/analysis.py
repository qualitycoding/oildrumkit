"""analysis.py -- shared audio-analysis helpers for the Oil Drum Kit A/B tooling.

All functions take a mono float array + sample rate. Nothing here depends on the plugin.
"""
import numpy as np
from scipy.io import wavfile
from scipy.signal import butter, sosfilt, find_peaks, hilbert

INSTRUMENTS = ["Bass Drum", "Snare 1", "Snare 2", "Low Tom 1", "Low Tom 2", "Mid Tom 1", "Mid Tom 2",
               "High Tom 1", "High Tom 2", "Splash", "Ride", "Hi-Hat Closed", "Hi-Hat Open", "Cowbell", "Rimshot"]
OCTAVE_CENTRES = [63, 125, 250, 500, 1000, 2000, 4000, 8000]


def load(path):
    """Return (sr, data[n, ch] float32). int16 is scaled to +-1."""
    sr, x = wavfile.read(path)
    if x.dtype == np.int16:
        x = x.astype(np.float32) / 32768.0
    else:
        x = x.astype(np.float32)
    if x.ndim == 1:
        x = x[:, None]
    return sr, x


def mono(x):
    return x.mean(axis=1) if x.ndim == 2 else x


def spectrum_db(x, sr, t0=0.0, t1=None, nfft=1 << 16):
    """Hann-windowed magnitude spectrum in dB (re: its own maximum). Returns (freqs, dB)."""
    i0 = int(t0 * sr)
    i1 = len(x) if t1 is None else min(len(x), int(t1 * sr))
    seg = np.asarray(x[i0:i1], dtype=np.float64)
    if len(seg) < 8:
        return np.array([0.0]), np.array([-200.0])
    w = np.hanning(len(seg))
    nfft = max(nfft, 1 << int(np.ceil(np.log2(len(seg)))))
    X = np.abs(np.fft.rfft(seg * w, nfft))
    f = np.fft.rfftfreq(nfft, 1.0 / sr)
    db = 20 * np.log10(X / (X.max() + 1e-30) + 1e-12)
    return f, db


def count_partials(x, sr, t0=0.05, dur=0.2, rel_db=-40.0, prominence_db=12.0, min_sep_hz=15.0,
                   fmin=30.0, fmax=12000.0, floor_db=15.0, floor_win_hz=600.0):
    """Number of resolved TONAL peaks in a Hann window [t0, t0+dur]:
       - within `rel_db` of the strongest peak,
       - prominence >= `prominence_db`,
       - at least `floor_db` above the local median spectrum (+-floor_win_hz/2). For a noise
         periodogram P(peak > 15 dB over the median) ~ 1e-10, so noise ripple is not counted."""
    from scipy.ndimage import median_filter
    f, db = spectrum_db(x, sr, t0, t0 + dur)
    if len(f) < 3:
        return 0
    binhz = f[1] - f[0]
    med = median_filter(db, size=max(3, int(floor_win_hz / binhz) | 1), mode="nearest")
    sel = (f >= fmin) & (f <= fmax)
    d = np.where(sel, db, -200.0)
    pk, _ = find_peaks(d, height=rel_db, prominence=prominence_db, distance=max(1, int(min_sep_hz / binhz)))
    pk = [p for p in pk if db[p] - med[p] >= floor_db]
    return int(len(pk))


def spectral_centroid(x, sr, t0=0.0, t1=0.1, fmax=16000.0):
    f, _ = spectrum_db(x, sr, t0, t1)
    i0 = int(t0 * sr); i1 = min(len(x), int(t1 * sr))
    seg = np.asarray(x[i0:i1], dtype=np.float64) * np.hanning(i1 - i0)
    nfft = 1 << int(np.ceil(np.log2(len(seg))))
    P = np.abs(np.fft.rfft(seg, nfft)) ** 2
    ff = np.fft.rfftfreq(nfft, 1.0 / sr)
    m = ff <= fmax
    return float((ff[m] * P[m]).sum() / (P[m].sum() + 1e-30))


def band_signal(x, sr, f0, edges=None):
    lo, hi = edges if edges else (f0 / np.sqrt(2), min(f0 * np.sqrt(2), 0.45 * sr))
    hi = min(hi, 0.45 * sr)
    sos = butter(4, [lo, hi], btype="band", fs=sr, output="sos")
    return sosfilt(sos, np.asarray(x, dtype=np.float64))


def band_t60(x, sr, f0, floor_db=-60.0, edges=None):
    """Octave-band RT60 via Schroeder backward integration. Returns seconds, or None if the
    band has < floor_db energy vs. the full signal or the decay range can't be measured."""
    y = band_signal(x, sr, f0, edges)
    e = y * y
    tot = float((np.asarray(x, dtype=np.float64) ** 2).sum()) + 1e-30
    if 10 * np.log10(e.sum() / tot + 1e-30) < floor_db + 20:      # < -40 dB of total energy
        return None
    edc = np.cumsum(e[::-1])[::-1]
    edc_db = 10 * np.log10(edc / edc[0] + 1e-30)
    t = np.arange(len(e)) / sr
    guard = int(0.9 * len(e))
    def fit(hi, lo):
        a = np.where(edc_db <= hi)[0]
        b = np.where(edc_db <= lo)[0]
        if len(a) == 0 or len(b) == 0 or b[0] > guard or b[0] - a[0] < 8:
            return None
        s = np.polyfit(t[a[0]:b[0]], edc_db[a[0]:b[0]], 1)[0]
        return -60.0 / s if s < 0 else None
    return fit(-5, -25) or fit(-5, -15)


def band_t60_table(x, sr):
    return {fc: band_t60(x, sr, fc) for fc in OCTAVE_CENTRES if fc * np.sqrt(2) < 0.45 * sr}


def xcorr_peak(a, b, maxlag=24):
    """Peak of the normalised cross-correlation of two equal-length segments over +-maxlag samples."""
    n = min(len(a), len(b)); a = np.asarray(a[:n], dtype=np.float64); b = np.asarray(b[:n], dtype=np.float64)
    na = np.sqrt((a * a).sum()) + 1e-30; nb = np.sqrt((b * b).sum()) + 1e-30
    best = -1.0
    for lag in range(-maxlag, maxlag + 1):
        if lag >= 0: c = (a[lag:] * b[:n - lag]).sum()
        else: c = (a[:n + lag] * b[-lag:]).sum()
        best = max(best, c / (na * nb))
    return float(best)


def highpass(x, sr, fc=1000.0):
    sos = butter(4, fc, btype="high", fs=sr, output="sos")
    return sosfilt(sos, np.asarray(x, dtype=np.float64))
