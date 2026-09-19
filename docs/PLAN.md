# PLAN — Oil Drum Kit realism rewrite (original, verbatim)

> **Status of this file.** This is the plan exactly as it was issued *before* implementation, reproduced
> verbatim and **deliberately not corrected**. It contains factual and numerical errors that were found
> during execution (see [`RETROSPECTIVE.md`](RETROSPECTIVE.md), which gives a per-step disposition:
> what was followed, changed, replaced or dropped, and why). Read them together.
>
> Provenance: written by Claude (Anthropic) in a chat session in response to the requirements in §0,
> derived from the analysis in Appendix A. Implementation branch: `modal-rewrite`.

---

## 0. Requirements as stated by the requester (verbatim)

Initial request:

> I have code on GitHub at https://github.com/qualitycoding/oildrumkit
> It's a vst of a drum kit made from oil drums. However I am not happy with the sounds that are produced as they do not sound very realistic to me.
> I want you to analyse the code and determine how to change the simulation so that the sounds produced by the vst are much more realistic and are like metal being struck by a drumstick

Planning request:

> Create a plan to address all of the changes suggested. The plan should include explicit detail of all the steps taken so it can be executed with total accuracy on a less capable sonnet model.
> The plan should include before and after wav file generation.
> It should include research to determine the mode tables

Execution request:

> Execute this plan

(Retrospective request, later: "Run a retrospective over the implementation of the plan … Record the plan and the retro output in all its detail in the repo alongside your implementation".)

---

## 1. The plan (verbatim)

# Oil Drum Kit — Realism Rewrite Plan

Execute phases in order. Each phase has a hard acceptance check; do not proceed if it fails.

---

## Phase 0 — Setup and offline renderer

**0.1** Branch: `git checkout -b modal-rewrite`

**0.2** Create `Tools/wav.h` — minimal 16-bit PCM WAV writer:
```cpp
#pragma once
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cmath>
inline void writeWav(const char* path, const std::vector<float>& L,
                     const std::vector<float>& R, int sr)
{
    const int n = (int)L.size(); const int ch = 2; const int bits = 16;
    const int dataBytes = n * ch * bits / 8;
    FILE* f = fopen(path, "wb");
    auto u32=[&](uint32_t v){fwrite(&v,4,1,f);}; auto u16=[&](uint16_t v){fwrite(&v,2,1,f);};
    fwrite("RIFF",1,4,f); u32(36+dataBytes); fwrite("WAVE",1,4,f);
    fwrite("fmt ",1,4,f); u32(16); u16(1); u16(ch); u32(sr);
    u32(sr*ch*bits/8); u16(ch*bits/8); u16(bits);
    fwrite("data",1,4,f); u32(dataBytes);
    for (int i=0;i<n;++i) for (int c=0;c<ch;++c) {
        float v = (c==0?L[i]:R[i]);
        v = v>1.f?1.f:(v<-1.f?-1.f:v);
        int16_t s = (int16_t)lrintf(v*32767.f); fwrite(&s,2,1,f);
    }
    fclose(f);
}
```

**0.3** Create `Tools/render_demo.cpp`. It must `#include "../Source/DrumEngine.h"` and `wav.h`, and render a **fixed, deterministic test pattern** at 48 kHz:

| Section | Content | Duration |
|---|---|---|
| A | Each of the 15 instruments, single hit, velocity 1.0, 1.2 s apart | 18 s |
| B | Each instrument, 4 hits at velocities 0.2, 0.45, 0.7, 1.0, 0.35 s apart | 21 s |
| C | 8-bar groove at 96 BPM (kick 1&3, snare 2&4, closed hat 8ths, tom fill bar 8) | 20 s |
| D | Snare roll, 32nd notes, 2 s (exposes machine-gunning) | 2 s |

Seed the engine's PRNG identically each run so A/B diffs are meaningful. Output path from `argv[1]`.

**0.4** Build script `Tools/build_render.sh`:
```bash
g++ -O2 -std=c++17 Tools/render_demo.cpp -o /tmp/render_demo && echo OK
```

**Acceptance 0:** `/tmp/render_demo /tmp/x.wav` produces a playable stereo WAV of ~61 s.

> Note: the current engine is mono. For Phase 0 write the same signal to L and R. Stereo arrives in Phase 6.

---

## Phase 1 — "Before" WAVs

**1.1** `mkdir -p Renders/before`
**1.2** Run: `/tmp/render_demo Renders/before/oildrumkit_before.wav`
**1.3** Split into per-section files `before_A.wav` … `before_D.wav` using the known sample offsets from 0.3.
**1.4** Create `Tools/spectra.py`:
```python
# usage: python3 spectra.py in.wav out.png [--t0 s] [--t1 s]
# produces: (a) log-log magnitude spectrum of the window,
#           (b) spectrogram 0-16 kHz,
#           (c) per-octave-band RT60 estimate via Schroeder backward integration
```
Use numpy, scipy.signal, matplotlib. Run it on each single-hit from section A of the *before* render; save PNGs to `Renders/before/`.
**1.5** Commit these — they are the baseline and must never be regenerated after the engine changes.

**Acceptance 1:** 5 before-WAVs + 15 spectrum PNGs committed.

---

## Phase 2 — Research: build the mode tables

Two independent tracks. Do **both**; track B validates track A.

### Track A — Analytic modes (compute these, do not guess)

Create `Tools/modes.py`. Reference geometry, a standard 55-gallon / 205 L steel drum:

| Symbol | Value |
|---|---|
| Outer radius R | 0.2858 m |
| Height L | 0.851 m |
| Wall thickness h | 1.2 mm (18 ga); head/end-cap h_c = 1.5 mm |
| E | 200 GPa |
| ρ | 7850 kg/m³ |
| ν | 0.30 |
| c (air) | 343 m/s |

**A1 — End cap (the struck surface): clamped circular plate.**
```
f_{d,c} = (λ²_{d,c} / (2π R²)) · sqrt( E h_c² / (12 ρ (1-ν²)) )
```
Frequency **ratios** relative to the (0,0) mode are fixed and must be used verbatim (d = nodal diameters, c = nodal circles):

```
(0,0) 1.000   (1,0) 2.080   (2,0) 3.410   (0,1) 3.890
(3,0) 5.000   (1,1) 5.950   (4,0) 6.820   (2,1) 8.280
(0,2) 8.720   (5,0) 8.840   (3,1) 10.90   (6,0) 11.10
(1,2) 13.00   (4,1) 13.70   (7,0) 13.70   (0,3) 15.70
```
Extend beyond this with the asymptotic plate law f ∝ (d + 2c + 1/2)². Generate ≥ 60 modes.

**A2 — Shell wall: Donnell thin-shell equation**, axial half-waves m, circumferential waves n:
```
λ = m π R / L,   β² = h² / (12 R²)
Ω² = [ (1-ν²) λ⁴ + β² (λ²+n²)⁴ ] / (λ² + n²)²
f   = Ω / (2π R) · sqrt( E / (ρ (1-ν²)) )
```
Sweep m = 1..8, n = 0..14. Sort, keep the lowest 80. **Expect the minimum near n = 4–6, not n = 0** — this non-monotonic behaviour is the signature that makes barrels sound like barrels, so verify it appears; if the curve is monotonic in n your β is wrong.

**A3 — Air cavity.** Open barrel = closed-open tube: `f_k = (2k-1)·c / (4(L + 0.3·2R))` → 84 Hz, 252 Hz, 420 Hz… Sealed barrel = Helmholtz on the bung hole; for the kit, use the open-tube set at amplitude 0.25 and T60 ≈ 0.25 s.

**A4 — Damping law.** For welded mild steel with radiation loading, decay rate σ (nepers/s, amplitude = e^{-σt}, T60 = 6.91/σ):
```
σ(f) = σ0 + σ1·f + σ2·f²
σ0 = 0.6      (air/mount losses, s⁻¹)
σ1 = 3.0e-3   (structural, η ≈ 2e-3, σ_struct = π η f)
σ2 = 1.2e-6   (thermoelastic + radiation)
```
Sanity targets: T60 ≈ 8 s at 80 Hz, ≈ 2 s at 800 Hz, ≈ 0.35 s at 5 kHz. Tune σ1, σ2 to hit these, then hold them fixed.

**A5** Emit `Source/ModeTables.h` — generated, do not hand-edit — containing for each of the 15 instruments a `static constexpr ModeSpec` array with fields `{ ratio, sigma0Hz, weight, m, n, kind }`. Cap at 96 modes for toms/bass/snare, 160 for hats/splash/ride.

### Track B — Empirical validation

**B1** Obtain 3–6 recordings of a real struck steel drum/barrel. Search Freesound for `oil drum hit`, `steel barrel strike`, `55 gallon drum percussion` (CC0/CC-BY only; record the licence in `Renders/sources.md`). Failing that, any struck sheet-steel or steelpan recording.
**B2** Add to `Tools/modes.py` a `--analyse` mode:
- Window the first 4 s after onset.
- Peak-pick a 65536-point FFT; keep peaks ≥ 35 dB below the max, min spacing 4 Hz.
- For each peak, band-filter ±1.5% and fit log-amplitude vs time by least squares → σ.
- Optionally refine with ESPRIT (`scipy.linalg`) on the first 0.5 s for close pairs.
- Write CSV: `freq_hz, sigma, rel_amp`.

**B3** Compare to Track A. Adjust h, R, L (not the physics) until the lowest 15 analytic frequencies land within ±8% of the measured set. Record the fitted geometry in a comment block at the top of `ModeTables.h`.

**B4 — Mode splitting.** In the measured data, near-degenerate pairs will show as 1–5 Hz beats. Set the per-instrument `splitCents` so `Δf/f` matches (typically 0.1–0.6 %). Every mode with n ≥ 1 becomes a **pair**.

**Acceptance 2:** `ModeTables.h` compiles standalone; `modes.py --report` prints, per instrument, mode count, f_min, f_max, T60 at 100/1k/5k Hz; the lowest 15 analytic modes match measurement within ±8%.

---

## Phase 3 — Core engine rewrite

Write a new `Source/DrumEngine.h`. **The public API must not change**: `DrumEngine(double)`, `setSampleRate`, `setHammer`, `setPitchHz`, `getPitchHz`, `trigger(int,float)`, `allNotesOff`, `getRecipe`, `process(float*,int)`. Keep `enum Instrument`, `enum HammerType`, `instrumentName`, `kNumInstruments`. Add an overload `process(float* L, float* R, int n)`; keep the mono one delegating to it. `PluginProcessor.cpp` must compile untouched except for Phase 6.4.

**3.1 Replace sine partials with two-pole resonators.** Per mode, at trigger:
```cpp
const float w = 2.0f * kPi * f / sr;
const float r = std::exp(-sigma / sr);
m.a1 = 2.0f * r * std::cos(w);
m.a2 = -r * r;
m.gain = weight * std::sin(w);   // unit-ish peak, cheap normalisation
m.y1 = m.y2 = 0.0f;
```
Per sample:
```cpp
const float y = m.a1*m.y1 + m.a2*m.y2 + m.gain*x;   // x = shared excitation
m.y2 = m.y1; m.y1 = y; acc += y;
```
No `sin()`, no per-mode envelope, no per-mode noise generator. This is cheaper than the current code and affords 10× the modes.

**3.2 Physical hammer pulse.** Replace `clickEnv`/`clickLpState` with a raised-cosine contact force of duration τ:
```cpp
// contact time shortens with velocity (Hertzian): τ = τ0 · v^(-0.2)
float tau = tau0[hammer] * std::pow(std::max(v, 0.05f), -0.2f);
// τ0: Metal 0.00035 s, Wood 0.00090 s, Rubber 0.00320 s
// force(t) = 0.5·(1 - cos(2π t / τ))  for 0 ≤ t < τ, else 0
```
Scale amplitude by `v`. Add a small scrape term: `+ 0.06 * noise * force(t)` for surface roughness. **Velocity must now change timbre, not just level** — this is the single biggest realism win.

**3.3 Frequency-dependent damping.** Compute σ from the A4 law at trigger time, per mode, per instrument (allow a per-instrument `dampScale` multiplier: bass 1.0, toms 1.3, hats 9.0, ride 0.45, rimshot 14.0 — these emulate hand/felt damping).

**3.4 Strike position and mode shapes.** Store `(m, n)` per mode. At trigger, draw a strike point:
```cpp
float rr = strikeR + 0.05f * urand();   // normalised radius, default 0.55
float th = strikeTheta + 0.35f * urand();
weight_eff = weight * J_approx(n, rr) * std::cos(n * th);
```
Use a small precomputed 16-point Bessel lookup for `J_approx`, or the cheap approximation `std::sin(kPi * (rr) * (d + 2c + 0.5f))`. Hitting near the rim must excite high modes far more than a centre hit; verify this audibly.

**3.5 Mode splitting / beating.** Every `n ≥ 1` mode instantiates twice at `f·(1 ± δ/2)` with `δ` drawn per hit from `splitCents ± 20 %`, and opposite-sign `cos(nθ)` weights. This produces the shimmer that is currently missing everywhere except Snare 2.

**3.6 Per-hit variation.** Jitter per trigger: strike position (3.4), split (3.5), ±0.4 % on all mode frequencies, ±8 % on τ, ±1.5 dB level. Rolls must stop machine-gunning.

**3.7 Kill the fake pitch envelope.** Set `pitchEnvAmt ≤ 0.05` everywhere, scaled by velocity: `amt = 0.05f * v`, decay 8 ms, applied as a real modulation of `a1` over the first 200 samples. Remove the 0.5/0.8 values entirely.

**3.8 Snare rattle — replace the bandpassed noise.** Model the snare wires as stochastic micro-impacts driven by shell amplitude:
```cpp
// each sample: p = clamp(rattleGain * |shellAccel|, 0, 1)
// if (urand01() < p * 0.35f) inject an impulse of amplitude ±0.3·p
//   into a 3-mode bright resonator bank at 2.6k / 4.1k / 7.3k Hz, Q ≈ 40
```
This gives the rattle a real attack/collapse envelope instead of a hiss fade.

**3.9 Metallic nonlinearity for hats/splash/ride.** Cymbal-like sheet steel shows energy cascading upward. Add per-voice:
```cpp
acc = acc + 0.12f * acc * acc * acc;    // mild cubic
acc = std::tanh(1.4f * acc) * 0.714f;   // soft ceiling
```
Apply to the cymbal-family voices only. Ride/splash should "bloom" rather than instantly decay.

**3.10 Air cavity.** Add one shared 2-pole resonator per instrument at the A3 frequency, fed by the hammer pulse through a lowpass at 300 Hz, amplitude 0.25, T60 0.25 s. Bass and toms only. This supplies the hollow thump.

**3.11 Fix the smoothing bug.** The current `currentHz` update runs once per `process()` call, so the effective time constant is ~20 ms × blockSize (≈10 s at 512 samples). Replace with:
```cpp
const float coef = std::exp(-(float)numSamples / (0.02f * (float)sampleRate));
currentHz[i] = targetHz[i] + coef * (currentHz[i] - targetHz[i]);
```
Also: **remove the live retune of ringing voices** (`v.hz = currentHz[inst]`). Real struck metal does not glide. Capture `hz` at trigger only. Keep the glide behind a compile-time `#define OILDRUM_LEGACY_GLIDE 0`.

**3.12 Voice/CPU budget.** Raise `kMaxInstances` to 8. Cap total active modes at 3000; when exceeded, steal from the oldest voice. Skip a mode's inner loop once `|y1| + |y2| < 1e-7`.

**Acceptance 3:** engine compiles with `-O2 -Wall -Wextra` with no new warnings; `render_demo` runs in under 0.25× real time; no NaN/Inf anywhere in output (assert in the renderer).

---

## Phase 4 — Per-instrument voicing

For each of the 15 instruments, set: base fundamental (keep the existing `defaultHz`/`minHz`/`maxHz` so the UI and `kParamSpecs` remain valid), which mode family (plate / shell / both), mode count, `dampScale`, `splitCents`, `strikeR`, cavity on/off, nonlinearity on/off.

Starting points — adjust by ear, do not invent new ones before A/B'ing:

| Instrument | Family | Modes | dampScale | strikeR | Cavity | NL |
|---|---|---|---|---|---|---|
| Bass | shell + cavity | 70 | 1.0 | 0.25 | yes | no |
| Snare 1/2 | plate + rattle | 80 | 2.2 | 0.55 | no | no |
| Toms ×6 | plate + shell + cavity | 64 | 1.3 | 0.45 | yes | no |
| Splash | plate | 140 | 5.0 | 0.85 | no | yes |
| Ride | plate | 160 | 0.45 | 0.70 | no | yes |
| Hats | plate | 120 | 9.0 | 0.90 | no | yes |
| Cowbell | shell, m=1 only | 24 | 3.0 | 0.60 | no | no |
| Rimshot | plate, rim strike | 48 | 14.0 | 0.97 | no | no |

**Acceptance 4:** all 15 sound distinct in section A; none clips at velocity 1.0 (peak ≤ −1 dBFS).

---

## Phase 5 — "After" WAVs and A/B

**5.1** `mkdir -p Renders/after`; rebuild `render_demo`; render `Renders/after/oildrumkit_after.wav` and the section splits.
**5.2** Run `spectra.py` on the matching after-hits; save PNGs.
**5.3** Create `Tools/compare.py` producing, for each instrument:
- before/after spectra overlaid,
- before/after spectrograms side by side,
- a table of mode count above −40 dB, spectral centroid vs time, and per-band T60.

**5.4 Numeric targets** (fail → return to Phase 3):
- Partials above −40 dB at t = 50 ms: before ≈ 4–6 → after ≥ 25.
- Spectral centroid at velocity 0.2 vs 1.0 must differ by ≥ 400 Hz (before: ~0).
- Successive hits in section D must be non-identical: cross-correlation peak ≤ 0.97.
- High-band (4–8 kHz) T60 must be < 25 % of low-band (80–200 Hz) T60.

**5.5** Where a real recording exists from B1, add it as a third column in `compare.py`.

**Acceptance 5:** all four numeric targets met; `Renders/README.md` written with the listening comparison and the licence notes from B1.

---

## Phase 6 — Stereo, space and polish

**6.1** Pan modes across the stereo field by `θ`: `panL = 0.5(1 + 0.6·cos θ)`. A real barrel radiates differently by angle; this alone widens the sound enormously.
**6.2** Add a short fixed early-reflection set (6 taps, 7–43 ms, −6 to −18 dB, slightly different L/R) plus a 1.2 s feedback-delay-network tail at 12 % wet. Industrial space is part of the identity.
**6.3** Add a soft bus limiter: `tanh` at −0.5 dBFS.
**6.4** `PluginProcessor.cpp`: change the two `scratch` buffers to `scratchL`/`scratchR`, call the stereo `process`, and `copyFrom` each channel separately. This is the only wrapper change required.
**6.5** Optional new parameters (append to `kParamSpecs`, never reorder existing entries — reordering breaks saved sessions): `strikePos` (0–1), `damping` (0.5–2.0), `roomMix` (0–0.4).

**Acceptance 6:** plugin builds as VST3; loads in a host; existing saved state from the old version still loads without crashing.

---

## Phase 7 — Commit and document

**7.1** Commit in this order: tools → mode tables → engine → voicing → renders. One phase per commit.
**7.2** Update `README.md`: replace the "mixture of sine oscillators and bandpassed noise" description with the modal-resonator/hammer-pulse model; cite the Donnell shell equation, clamped-plate ratios, and the measured-mode source.
**7.3** Add `Renders/` links for before/after.

---

### Rollback points
Phases 1, 2 and 5 produce artefacts that are never overwritten. If Phase 3 goes badly, `git checkout main -- Source/DrumEngine.h` restores the old engine while keeping `ModeTables.h` and the tooling.

---

## Appendix A — The analysis the plan was derived from (verbatim)

I read `DrumEngine.h` and the plugin wrapper. The engine is structured like an 808-style drum synth rather than a struck-metal model, and that's why it doesn't sound real. Here are the causes, ranked by impact.

**Why it sounds synthetic**
1. **Too few, too clean modes.** Each drum has 4-5 exact-ratio sines. A struck barrel has dozens to hundreds of inharmonic modes, and the high ones die within milliseconds.
2. **Noise instead of resonance.** The `N()` partials are continuously driven bandpassed noise with an envelope, which sounds like hiss. In real metal a short impact excites resonators that then ring on their own, and there is no continuous noise source.
3. **Excitation is just a noise click.** A real stick contact is a smooth 0.3-1.5 ms pulse. Harder or faster hits are shorter and brighter (contact time falls roughly as v^-0.2). Here velocity only scales amplitude, so soft and hard hits have the same timbre.
4. **Decay times are hand-picked.** Real damping is frequency-dependent, roughly σ = σ₀ + σ₁f², so highs vanish much faster than lows.
5. **The pitch envelope is wrong for steel.** Its 0.5-0.8× sweep is the classic 808 kick/tom trick. Real metal only glides a few percent, and only on hard hits.
6. **No beating or variation.** Real barrels have mode pairs split by 1-5 Hz (welds, seams, rims), which gives the shimmer. Only Snare 2 has one pair. Every hit is also identical apart from ±15% on the click, so rolls sound like machine-gunning.
7. **Mono, dry and no air cavity.** The body has no low sealed-cavity thump, and there's no stereo or room.

**What to change**
- **Replace the sine + envelope with impulse-excited damped resonators.** They're cheaper than `sin()` and let you afford 30-100 modes:
  ```cpp
  // per mode, at trigger
  w = 2π f / sr;  r = exp(-σ / sr);   // σ = σ0 + σ1 f²
  a1 = 2 r cos w;  a2 = -r*r;
  // per sample
  y = a1*y1 + a2*y2 + g * x;  y2 = y1;  y1 = y;
  ```
  Here `x` is one shared hammer pulse, and `g` is roughly (mode-shape weight at the strike point) × (pulse spectrum at f) / sin(w). Typical values: about 1 s T60 for low modes, about 0.1 s at 5 kHz.
- **Build the hammer as a half-sine pulse** with its length tied to velocity and hammer type. Feed that same pulse to all modes, so velocity changes brightness naturally. Keep a tiny transient tick for the stick click.
- **Split every mode into a pair** with a 0.1-0.5% detune and randomize it slightly per hit. Also jitter the strike position, which changes the mode weights.
- **Use different mode tables per family.** Toms and bass get the head/shell modes plus a low air-cavity mode, using plate-like spacing (roughly f ∝ n^1.8-2, not integer ratios). Hats, splash and ride get 60-150 randomized modes with fast-decaying highs and a mild soft-clip for crash-like buildup. Snares get the shell modes plus a rattle made of stochastic micro-impacts triggered by shell amplitude, instead of continuous noise.
- **Cut `pitchEnvAmt` to ≤0.05** and scale it with velocity.
- **For genuine realism, use measured or computed mode tables.** Record a real barrel and peak-pick the frequencies and T60s from an FFT. Or solve the plate/shell eigenproblem offline and bake the table into the recipe.

**A bug affecting pitch accuracy**
In `process()`, the `currentHz` smoothing runs once per call, not per sample. The time constant is therefore about 20 ms × the block length, or roughly 10 s at 512 samples. Knob changes lag badly and drums trigger at stale pitches. Compute the smoothing per block with the block length in the exponent, or drop it and just tune at trigger.
