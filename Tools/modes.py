#!/usr/bin/env python3
"""modes.py -- physics-derived mode tables for the Oil Drum Kit engine.

  modes.py verify                     # test the solvers against published values
  modes.py generate                   # write Source/ModeTables.h and Source/Voicing.h
  modes.py report [--sr 48000]        # per-instrument mode count, f_min/f_max, T60 at 100/1k/5k
  modes.py analyse FILE.wav [--t0 S --dur S --floor DB --out CSV]   # empirical mode extraction

Model (see Renders/RESEARCH.md for sources and confidence levels):
  * STRUCK LID = clamped circular plate. Exact eigenvalues from  J_n(l) I_{n+1}(l) + I_n(l) J_{n+1}(l) = 0
    (verified against a published table and the Blevins/ANSYS VM181 case). Mode shapes
    W(r) = J_n(l r) - [J_n(l)/I_n(l)] I_n(l r), tabulated on 48 radii.
  * SIDE WALL = Donnell thin-shell equation, simply-supported ends, modes (m, n).
  * AIR CAVITY = closed rigid cylinder modes (axial / circumferential / radial).
  * Every frequency is stored as a RATIO to the reference lid fundamental (100 Hz, measured on real
    55-gal drum lids, US 6,339,960). A drum with UI pitch `hz` is a geometrically-similar scaled drum:
    f = hz * ratio.
"""
import argparse, json, math, os, sys
import numpy as np
from scipy.special import jv, ive, jn_zeros, jnp_zeros
from scipy.optimize import brentq

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

# ---- reference 55-gallon drum (steel) ---------------------------------------------------------
R_DRUM, L_DRUM = 0.2858, 0.851          # m  (outer radius, wall height)
H_WALL = 1.2e-3                          # m  (18 gauge)
E_STEEL, RHO_STEEL, NU = 200e9, 7850.0, 0.30
C_AIR = 343.0
F_LID_REF = 100.0                        # Hz, measured fundamental of real 55-gal lids (US 6,339,960)

# ---- damping law (engine): sigma(f) = sigma0 + pi*eta*f + sigma2*f^2   [nepers/s; T60 = 6.91/sigma]
# eta range 0.001 (bare steel, 'clang') .. 0.01 ('bong') is sourced (see RESEARCH.md); the chosen
# point inside that range and sigma0/sigma2 are ENGINEERING ESTIMATES, not measurements.
DAMP = dict(sigma0=0.5, eta=0.0035, sigma2=1.0e-6)

N_PLATE, N_SHELL, N_CAV, N_PROF = 200, 80, 8, 48

# Published clamped-plate table (lambda^2). ORIENTATION: row index = nodal CIRCLES s (0..6),
# column index = nodal DIAMETERS n (0..5).  (Bessel order n = diameters.)
# Source: circular plate clamped all around, Bessel frequency-equation solution (studia.reviste.ubbcluj.ro)
PUBLISHED_L2 = {
    0: [10.21583, 21.2604, 34.87704, 51.03004, 69.66583, 90.73899],
    1: [39.77115, 60.82867, 84.58265, 111.0214, 140.1079, 171.8030],
    2: [89.10414, 120.0792, 153.8151, 190.3038, 229.5186, 271.4282],
    3: [158.1842, 199.0534, 242.7206, 289.1799, 338.4112, 390.3895],
    4: [247.0064, 297.7601, 351.3360, 407.7295, 466.9250, 528.9021],
    5: [355.5693, 416.2026, 479.6751, 545.9830, 615.1140, 687.0511],
    6: [483.8722, 554.3824, 627.7441, 703.9546, 783.0036, 864.8769],
}


def T60(sigma):
    return 6.9078 / sigma


def sigma_of(f, scale=1.0):
    return scale * (DAMP["sigma0"] + math.pi * DAMP["eta"] * f + DAMP["sigma2"] * f * f)


# ---- clamped circular plate -------------------------------------------------------------------
def _plate_F(n, lam):
    return jv(n, lam) * ive(n + 1, lam) / ive(n, lam) + jv(n + 1, lam)


def plate_roots(n, lam_max, dl=0.005):
    lam = np.arange(max(0.2, 0.9 * n), lam_max, dl)
    v = _plate_F(n, lam)
    idx = np.where(np.sign(v[:-1]) * np.sign(v[1:]) < 0)[0]
    return [brentq(lambda l: _plate_F(n, l), lam[i], lam[i + 1], xtol=1e-13) for i in idx]


def plate_profile(n, lam, npts=N_PROF):
    """Radial mode shape on r=i/(npts-1); normalised so that W(r)cos(n theta) has area mean-square 1."""
    def W(r):
        r = np.asarray(r, dtype=np.float64)
        ratio = jv(n, lam) * ive(n, lam * r) / ive(n, lam) * np.exp(-lam * (1.0 - r))
        return jv(n, lam * r) - ratio
    fine = np.linspace(0, 1, 8001)
    wf = W(fine)
    ms = np.trapezoid(wf * wf * 2 * fine, fine)                # area mean of W^2
    target = 1.0 if n == 0 else 2.0                              # cos^2 averages to 1/2
    scale = math.sqrt(target / ms)
    return (W(np.linspace(0, 1, npts)) * scale)


def build_plate_table(count=N_PLATE):
    lam_max = 48.0
    modes = []
    for n in range(0, 56):
        for s, l in enumerate(plate_roots(n, lam_max)):
            modes.append((l * l, n, s, l))
    modes.sort()
    l2_00 = modes[0][0]
    out = []
    for l2, n, s, l in modes[:count]:
        out.append(dict(ratio=l2 / l2_00, n=n, s=s, lam=l, prof=plate_profile(n, l)))
    return out, l2_00, modes[count - 1][0]


# ---- Donnell shell ----------------------------------------------------------------------------
def shell_freq(m, n, R=R_DRUM, L=L_DRUM, h=H_WALL):
    lam = m * math.pi * R / L
    beta2 = h * h / (12 * R * R)
    k2 = lam * lam + n * n
    Om2 = (1 - NU ** 2) * lam ** 4 / (k2 * k2) + beta2 * k2 * k2
    cl = math.sqrt(E_STEEL / (RHO_STEEL * (1 - NU ** 2)))
    return math.sqrt(Om2) * cl / (2 * math.pi * R)


def build_shell_table(count=N_SHELL):
    ms = [(shell_freq(m, n), m, n) for m in range(1, 13) for n in range(0, 21)]
    ms.sort()
    return [dict(ratio=f / F_LID_REF, m=m, n=n, f=f) for f, m, n in ms[:count]]


# ---- closed-cylinder air cavity ---------------------------------------------------------------
def build_cavity_table(count=N_CAV):
    out = []
    for n in range(0, 4):
        zs = [0.0] + list(jn_zeros(1, 3)) if n == 0 else list(jnp_zeros(n, 3))
        # for n>=1 the q=0 branch is the first zero of J_n'
        for q, x in enumerate(zs[:3]):
            for p in range(0, 5):
                if n == 0 and q == 0 and p == 0:
                    continue
                f = (C_AIR / (2 * math.pi)) * math.sqrt((x / R_DRUM) ** 2 + (p * math.pi / L_DRUM) ** 2)
                out.append((f, n, q, p))
    out.sort()
    return [dict(ratio=f / F_LID_REF, f=f, n=n, q=q, p=p) for f, n, q, p in out[:count]]


# ---- verify -----------------------------------------------------------------------------------
def cmd_verify():
    ok = True
    # 1. plate eigenvalues vs published table
    worst, cnt = 0.0, 0
    for s_circ, row in PUBLISHED_L2.items():          # row = circles s
        for n_diam, ref in enumerate(row):            # col = diameters n
            roots = plate_roots(n_diam, 32.0)
            if s_circ >= len(roots) or ref > 1000: continue
            worst = max(worst, abs(roots[s_circ] ** 2 / ref - 1)); cnt += 1
    print(f"[1] clamped-plate lambda^2 vs published table ({cnt} values, s=0..6 circles x n=0..5 diameters): worst rel. error {worst:.2e}")
    ok &= worst < 2e-5
    # 2. Blevins / ANSYS VM181: R=17in h=0.5in E=3e7psi rho=7.3e-4 lbf s^2/in^4 nu=0.3 -> 172.64 Hz
    R, h, E, rho = 17 * 0.0254, 0.5 * 0.0254, 3e7 * 6894.757, 7.3e-4 * 1.0688e7
    f = 10.2158 / (2 * math.pi * R * R) * math.sqrt(E * h * h / (12 * rho * (1 - NU ** 2)))
    print(f"[2] Blevins/ANSYS VM181 clamped plate f00: {f:.2f} Hz (published 172.64 Hz)")
    ok &= abs(f - 172.64) < 0.5
    # 3. shell: Donnell frequency should be NON-monotonic in n (minimum at n>0) and n=0 above ring frequency
    fm1 = [shell_freq(1, n) for n in range(0, 16)]
    nmin = int(np.argmin(fm1))
    f_ring = math.sqrt(E_STEEL / RHO_STEEL) / (2 * math.pi * R_DRUM)
    print(f"[3] Donnell m=1: min at n={nmin} ({fm1[nmin]:.0f} Hz), n=0 -> {fm1[0]:.0f} Hz (ring freq {f_ring:.0f} Hz)")
    ok &= 2 <= nmin <= 9 and fm1[0] > 0.9 * f_ring
    # 4. orthonormality sanity: profile mean-square
    p = plate_profile(2, plate_roots(2, 12)[0])
    print(f"[4] plate profile max |W| for (n=2,s=0): {np.abs(p).max():.3f} (finite, edge {abs(p[-1]):.1e} ~ 0 clamped)")
    ok &= abs(p[-1]) < 1e-6
    # 5. modal density (Weyl): N(lambda^2 <= L2) ~ L2/4
    tab, l2_00, l2_last = build_plate_table()
    n_eff = sum(1 if m["n"] == 0 else 2 for m in tab)      # n>=1 frequencies are cos/sin degenerate pairs
    print(f"[5] plate table: {len(tab)} frequencies ({n_eff} counting cos/sin pairs) up to ratio {tab[-1]['ratio']:.1f}; "
          f"Weyl N(L2) = L2/4 = {l2_last/4:.0f}")
    ok &= abs(l2_last / 4 - n_eff) < 0.12 * n_eff
    # 6. measured lid data (US 6,339,960): f1 ~ 100 Hz, second axisymmetric 250-280 Hz
    tab6 = build_plate_table()[0]
    r01 = next(m["ratio"] for m in tab6 if m["n"] == 0 and m["s"] == 1)
    print(f"[6] ideal clamped-plate (n=0,s=1)/(0,0) = {r01:.3f}. Measured real lids (US 6,339,960): 250-280/100 = 2.5-2.8 "
          f"(shallow ring), ~450/112 = 4.0 (deep ring).")
    print(f"    stretch s in r' = 1 + s(r-1): shallow-ring -> s={(2.65-1)/(r01-1):.2f}, deep-ring -> s={(4.0-1)/(r01-1):.2f}; engine default 0.8")
    print("VERIFY:", "PASS" if ok else "FAIL")
    return 0 if ok else 1


# ---- header generation ------------------------------------------------------------------------
def fmt(x):
    """C++ float literal: always has a '.' or exponent; values < 1e-20 flush to 0.0 (no denormal constants)."""
    if abs(x) < 1e-20:
        return "0.0f"
    t = f"{x:.7g}"
    if "." not in t and "e" not in t and "n" not in t:
        t += ".0"
    return t + "f"


def cmd_generate():
    plate, l2_00, _ = build_plate_table()
    shell = build_shell_table()
    cav = build_cavity_table()
    L = []
    A = L.append
    A("// ModeTables.h -- GENERATED by Tools/modes.py. DO NOT EDIT BY HAND.")
    A("//")
    A("// Physics-derived modal data for the oil-drum engine. All frequencies are RATIOS to the reference")
    A(f"// lid fundamental ({F_LID_REF:g} Hz, measured on real 55-gal drum lids). See Renders/RESEARCH.md.")
    A(f"// Reference drum: R={R_DRUM} m, L={L_DRUM} m, wall h={H_WALL*1e3:g} mm, steel E={E_STEEL/1e9:g} GPa rho={RHO_STEEL:g} nu={NU}.")
    A("// Plate: clamped circular plate (exact Bessel eigenvalues). Shell: Donnell thin-shell, simply-supported ends.")
    A("// Cavity: closed rigid cylinder. Radial plate profiles are sampled at r = i/(kProfilePts-1).")
    A("#pragma once")
    A("namespace oildrum { namespace tables {")
    A("")
    A(f"inline constexpr float kRefLidHz = {fmt(F_LID_REF)};")
    A(f"inline constexpr float kRefRadiusM = {fmt(R_DRUM)};   // reference drum radius")
    A(f"inline constexpr float kSpeedOfSound = {fmt(C_AIR)};")
    A("// Damping law: sigma(f) = sigma0 + pi*eta*f + sigma2*f^2  [1/s];  T60 = 6.9078/sigma")
    A(f"inline constexpr float kSigma0 = {fmt(DAMP['sigma0'])};")
    A(f"inline constexpr float kEta    = {fmt(DAMP['eta'])};")
    A(f"inline constexpr float kSigma2 = {fmt(DAMP['sigma2'])};")
    A("")
    A(f"inline constexpr int kProfilePts = {N_PROF};")
    A(f"inline constexpr int kNumPlateModes = {len(plate)};")
    A(f"inline constexpr int kNumShellModes = {len(shell)};")
    A(f"inline constexpr int kNumCavityModes = {len(cav)};")
    A("")
    A("struct PlateMode { float ratio; int n; int s; float prof[kProfilePts]; };   // n = nodal diameters, s = nodal circles")
    A("struct ShellMode { float ratio; int m; int n; };                            // m = axial half-waves, n = circumferential waves")
    A("struct CavityMode { float ratio; int n; int q; int p; };")
    A("")
    A("inline constexpr PlateMode kPlate[kNumPlateModes] = {")
    for m in plate:
        prof = ", ".join(fmt(v) for v in m["prof"])
        A(f"  {{ {fmt(m['ratio'])}, {m['n']}, {m['s']}, {{ {prof} }} }},")
    A("};")
    A("")
    A("inline constexpr ShellMode kShell[kNumShellModes] = {")
    for m in shell:
        A(f"  {{ {fmt(m['ratio'])}, {m['m']}, {m['n']} }},")
    A("};")
    A("")
    A("inline constexpr CavityMode kCavity[kNumCavityModes] = {")
    for m in cav:
        A(f"  {{ {fmt(m['ratio'])}, {m['n']}, {m['q']}, {m['p']} }},")
    A("};")
    A("")
    A("} } // namespace oildrum::tables")
    open(os.path.join(ROOT, "Source", "ModeTables.h"), "w").write("\n".join(L) + "\n")
    print(f"wrote Source/ModeTables.h  (plate {len(plate)} modes to ratio {plate[-1]['ratio']:.1f}; shell {len(shell)} "
          f"({shell[0]['f']:.0f}-{shell[-1]['f']:.0f} Hz ref); cavity {len(cav)} ({cav[0]['f']:.0f}-{cav[-1]['f']:.0f} Hz ref))")

    # ---- Voicing.h from voicing.json
    V = json.load(open(os.path.join(HERE, "voicing.json")))
    d = V["defaults"]
    fields = [("defaultHz", "float"), ("minHz", "float"), ("maxHz", "float"),
              ("plateModes", "int"), ("plateSkip", "int"), ("plateAmp", "float"), ("plateStretch", "float"),
              ("shellModes", "int"), ("shellAmp", "float"), ("shellXs", "float"),
              ("cavModes", "int"), ("cavAmp", "float"), ("cavT60", "float"),
              ("dampScale", "float"), ("splitPct", "float"), ("pairModes", "int"),
              ("strikeR", "float"), ("strikeRJit", "float"), ("tauScale", "float"), ("scrape", "float"),
              ("pitchEnvAmt", "float"), ("pitchEnvDecayMs", "float"),
              ("rattle", "float"), ("nl", "float"), ("residual", "float"), ("pan", "float"), ("chokeGroup", "int"), ("outGain", "float")]
    W = []
    B = W.append
    B("// Voicing.h -- GENERATED by Tools/modes.py from Tools/voicing.json. DO NOT EDIT BY HAND.")
    B("#pragma once")
    B("namespace oildrum {")
    B("")
    B("// Per-instrument recipe. `defaultHz`/`minHz`/`maxHz` mirror the UI parameter ranges in PluginProcessor.cpp.")
    B("struct Recipe")
    B("{")
    for name, ty in fields:
        B(f"    {ty} {name};")
    B("};")
    B("")
    B(f"inline constexpr int kNumInstruments = {len(V['instruments'])};")
    B("")
    B("inline constexpr Recipe kVoicing[kNumInstruments] = {")
    for ins in V["instruments"]:
        vals = []
        for name, ty in fields:
            v = ins.get(name, d.get(name))
            vals.append(str(int(v)) if ty == "int" else fmt(float(v)))
        B(f"  /* {ins['name']:14s} */ {{ " + ", ".join(vals) + " },")
    B("};")
    B("")
    B("inline const char* instrumentName (int i)")
    B("{")
    B("    static const char* names[kNumInstruments] = {")
    B("        " + ", ".join(f'"{i["name"]}"' for i in V["instruments"]))
    B("    };")
    B("    return (i >= 0 && i < kNumInstruments) ? names[i] : \"?\";")
    B("}")
    B("")
    B("} // namespace oildrum")
    open(os.path.join(ROOT, "Source", "Voicing.h"), "w").write("\n".join(W) + "\n")
    print("wrote Source/Voicing.h     (", len(V["instruments"]), "instruments )")
    return 0


# ---- report -----------------------------------------------------------------------------------
def instrument_modes(ins, d, plate, shell, cav, sr):
    """Absolute-frequency mode list [(f, family)] for an instrument (before pair-splitting)."""
    get = lambda k: ins.get(k, d.get(k))
    hz, st = get("defaultHz"), get("plateStretch")
    nyq = 0.45 * sr
    out = []
    sk = int(get("plateSkip"))
    anchor = 1 + st * (plate[sk]["ratio"] - 1)          # first KEPT plate mode is placed at `hz`
    for m in plate[sk:sk + int(get("plateModes"))]:
        f = hz * (1 + st * (m["ratio"] - 1)) / anchor
        if f < nyq: out.append((f, "plate"))
    for m in shell[:int(get("shellModes"))]:
        f = hz * m["ratio"]
        if f < nyq: out.append((f, "shell"))
    for m in cav[:int(get("cavModes"))]:
        f = hz * m["ratio"]
        if f < nyq: out.append((f, "cavity"))
    return out


def cmd_report(sr):
    plate, _, _ = build_plate_table(); shell = build_shell_table(); cav = build_cavity_table()
    V = json.load(open(os.path.join(HERE, "voicing.json"))); d = V["defaults"]
    print(f"damping law: sigma = {DAMP['sigma0']} + pi*{DAMP['eta']}*f + {DAMP['sigma2']:g}*f^2   (dampScale multiplies sigma)")
    print(f"{'instrument':14s} {'hz':>5s} {'modes':>5s} {'(P/S/C)':>10s} {'f_min':>7s} {'f_max':>7s} {'T60@100':>8s} {'T60@1k':>8s} {'T60@5k':>8s}")
    for ins in V["instruments"]:
        ms = instrument_modes(ins, d, plate, shell, cav, sr)
        cnt = {k: sum(1 for _, fam in ms if fam == k) for k in ("plate", "shell", "cavity")}
        pair = int(ins.get("pairModes", d["pairModes"]))
        paired = sum(1 for i, (f, fam) in enumerate(ms) if fam == "plate" and i < pair)
        ds = ins.get("dampScale", d["dampScale"])
        t = lambda f: T60(sigma_of(f, ds))
        fs = [f for f, _ in ms]
        print(f"{ins['name']:14s} {ins['defaultHz']:5.0f} {len(ms):5d} {cnt['plate']:>3d}/{cnt['shell']:>2d}/{cnt['cavity']:>1d}   {min(fs):7.0f} {max(fs):7.0f} "
              f"{t(100):8.2f} {t(1000):8.3f} {t(5000):8.3f}   (+{paired} split partners)")
    return 0


# ---- empirical analysis (Track B) -------------------------------------------------------------
def analyse(x, sr, t0=0.0, dur=4.0, floor_db=-35.0, min_sep=4.0, band_frac=0.015):
    """Peak-pick a long FFT of x[t0:t0+dur]; for each peak fit the decay rate of its band-limited
    Hilbert envelope. Returns list of (freq_hz, sigma_1/s, t60_s, rel_amp_db)."""
    from scipy.signal import find_peaks, butter, sosfilt, hilbert
    seg = np.asarray(x[int(t0 * sr):int((t0 + dur) * sr)], dtype=np.float64)
    nfft = 1 << (int(math.ceil(math.log2(len(seg)))) + 1)
    # FALLING half-Hann: w(0)=1, w(end)=0. A full Hann is ~0 at the onset and would erase fast-decaying modes.
    X = np.abs(np.fft.rfft(seg * np.hanning(2 * len(seg))[len(seg):], nfft))
    f = np.fft.rfftfreq(nfft, 1 / sr)
    db = 20 * np.log10(X / X.max() + 1e-12)
    binhz = f[1]
    pk, _ = find_peaks(db, height=floor_db, distance=max(1, int(min_sep / binhz)), prominence=6)
    res = []
    for p in pk:
        fc = f[p]
        if fc < 20 or fc > 0.45 * sr: continue
        lo, hi = fc * (1 - band_frac), min(fc * (1 + band_frac), 0.49 * sr)
        sos = butter(4, [lo, hi], btype="band", fs=sr, output="sos")
        y = sosfilt(sos, seg)
        env = np.abs(hilbert(y))
        i0 = int(np.argmax(env[: int(0.05 * sr) + 1]))
        e = env[i0:]
        top = e[: max(8, int(0.005 * sr))].max()
        good = np.where(e > top * 10 ** (-40 / 20))[0]
        if len(good) < int(0.02 * sr): continue
        end = good[-1] if len(good) else len(e) - 1
        # fit to the initial monotone-ish decay: up to the first time env < top*10^(-30/20)
        below = np.where(e < top * 10 ** (-30 / 20))[0]
        end = below[0] if len(below) else end
        if end < int(0.02 * sr): continue
        tt = np.arange(end) / sr
        slope = np.polyfit(tt, np.log(e[:end] + 1e-30), 1)[0]
        sigma = -slope
        if sigma <= 0: continue
        res.append((float(fc), float(sigma), float(T60(sigma)), float(db[p])))
    return res


def cmd_analyse(a):
    sys.path.insert(0, HERE)
    import analysis as A
    sr, x = A.load(a.file); x = A.mono(x)
    res = analyse(x, sr, a.t0, a.dur, a.floor)
    out = a.out or (os.path.splitext(a.file)[0] + "_modes.csv")
    with open(out, "w") as fh:
        fh.write("freq_hz,sigma,t60_s,rel_amp_db\n")
        for r in res: fh.write("%.3f,%.4f,%.4f,%.2f\n" % r)
    print(f"{len(res)} modes -> {out}")
    for r in res[:25]: print("  %9.2f Hz   sigma %8.3f   T60 %6.3f s   %6.1f dB" % r)
    return 0


def main():
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("verify"); sub.add_parser("generate")
    r = sub.add_parser("report"); r.add_argument("--sr", type=int, default=48000)
    an = sub.add_parser("analyse"); an.add_argument("file"); an.add_argument("--t0", type=float, default=0.0)
    an.add_argument("--dur", type=float, default=4.0); an.add_argument("--floor", type=float, default=-35.0)
    an.add_argument("--out", default=None)
    a = ap.parse_args()
    sys.exit({"verify": lambda: cmd_verify(), "generate": lambda: cmd_generate(),
              "report": lambda: cmd_report(a.sr), "analyse": lambda: cmd_analyse(a)}[a.cmd]())


if __name__ == "__main__":
    main()
