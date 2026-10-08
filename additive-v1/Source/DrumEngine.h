// DrumEngine.h
//
// Portable, header-only synthesis engine for an "oil drum" kit.
// No external dependencies (no JUCE; only <cmath>/<array>/<cstdint>/<algorithm>), so this
// exact file is used by the unit tests, the offline renderers and the JUCE plugin.
//
// SOUND MODEL
//  * Drums, cowbell, rimshot: a bank of "partials". Each partial is a sine oscillator
//    (shell resonance) or a resonant bandpassed-noise stream (rattle / snare wires). Partial
//    frequencies are ratios of the instrument's tunable fundamental, deliberately inharmonic,
//    so the result reads as struck sheet steel. Toms and the bass drum also sweep their sine
//    partials down in pitch for the first ~40 ms (pitch envelope) for punch.
//  * Cymbals and hi-hats ("metal" block): six band-limited (PolyBLEP) square oscillators at
//    inharmonic ratios plus a noise wash, through a 24 dB/oct high-pass, with an exponential
//    decay. The ratios are those recorded in knowledge item K-20261004T162314Z; their
//    attribution to a particular drum machine is UNVERIFIED.
//  * Every hit also gets a short broadband noise "click" for the stick attack.
//
// SIGNAL PATH (the only way audio leaves the engine):
//    voices -> renderRaw() (sum, no gain) -> render(): master gain (kMasterGain) -> tanh soft clip
//  render() output is bounded to |x| < 1 for any input. renderRaw() is for tests and measurement.
//
// PITCH: setPitchHz() sets the tuning used by the NEXT trigger(). A ringing note keeps the
// pitch it was struck at (measured at S-RETRO and re-tested in tests/test_engine.cpp).
//
// Allocation-free after construction; safe to call from a real-time audio thread.

#pragma once
#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>

namespace oildrum
{

constexpr int kMaxPartials = 8;
constexpr int kMaxInstances = 4;      // overlapping hits per instrument (rolls / flams)
constexpr int kNumInstruments = 15;
constexpr int kMetalOscs = 6;

// Master gain applied inside render(). The loudest tested case (Bass + Snare 1 + Ride + Low Tom 1
// struck together) peaks well above 1.0 before this stage; see tests/test_engine.cpp.
constexpr float kMasterGain = 0.25f;

enum Instrument
{
    Bass = 0,
    Snare1,
    Snare2,
    TomLow1,
    TomLow2,
    TomMid1,
    TomMid2,
    TomHigh1,
    TomHigh2,
    Splash,
    Ride,
    HihatClosed,
    HihatOpen,
    Cowbell,
    Rimshot
};

inline const char* instrumentName (int i)
{
    static const char* names[kNumInstruments] = {
        "Bass Drum", "Snare 1", "Snare 2",
        "Low Tom 1", "Low Tom 2", "Mid Tom 1", "Mid Tom 2", "High Tom 1", "High Tom 2",
        "Splash", "Ride", "Hi-Hat Closed", "Hi-Hat Open", "Cowbell", "Rimshot"
    };
    return (i >= 0 && i < kNumInstruments) ? names[i] : "?";
}

// Inharmonic frequency ratios of the metallic square bank (K-20261004T162314Z).
constexpr std::array<float, kMetalOscs> kMetalRatios = { 1.0f, 1.4471f, 1.6170f, 1.9265f, 2.5028f, 2.6637f };

// ---------------------------------------------------------------------------
// One resonant partial in a drum's recipe.
struct Partial
{
    float ratio  = 1.0f;   // frequency = tuning Hz * ratio
    float amp    = 0.0f;   // 0..~1
    float decay  = 0.2f;   // seconds; exponential time constant (amplitude falls to 1/e)
    bool  noise  = false;  // false = sine oscillator, true = resonant bandpass noise
    float q      = 10.0f;  // bandpass Q, only used when noise == true
};

// Metallic voice used by cymbals and hi-hats. amp == 0 disables it.
struct Metal
{
    float amp        = 0.0f;
    float decay      = 0.3f;     // seconds, exponential time constant
    float baseScale  = 0.6f;     // oscillator base frequency = tuning Hz * baseScale
    float hpHz       = 6000.0f;  // 24 dB/oct high-pass corner
    float noiseMix   = 0.35f;    // white-noise wash level relative to the square bank
    float lpHz       = 12000.0f; // 12 dB/oct low-pass corner (tames the top octave)
};

// A full instrument recipe.
struct Recipe
{
    float defaultHz = 200.0f;
    float minHz     = 40.0f;
    float maxHz     = 800.0f;
    std::array<Partial, kMaxPartials> partials{};
    Metal metal{};
    float clickAmp      = 0.15f;   // broadband attack transient
    float clickDecay    = 0.006f;  // seconds
    float pitchEnvAmt   = 0.0f;    // extra frequency multiplier at t=0 (sine partials only)
    float pitchEnvDecay = 0.03f;   // seconds
    int   chokeGroup    = 0;       // instruments sharing a nonzero group choke each other
};

// ---------------------------------------------------------------------------
// RBJ biquad (bandpass: constant skirt gain, peak = Q; highpass).
struct Biquad
{
    float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    void setBandpass (float freqHz, float q, float sampleRate)
    {
        freqHz = std::clamp (freqHz, 20.0f, sampleRate * 0.45f);
        const float w0 = 2.0f * float (M_PI) * freqHz / sampleRate;
        const float alpha = std::sin (w0) / (2.0f * std::max (q, 0.05f));
        const float cosw0 = std::cos (w0);
        const float a0 = 1.0f + alpha;
        b0 =  alpha / a0;
        b1 =  0.0f;
        b2 = -alpha / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void setHighpass (float freqHz, float q, float sampleRate)
    {
        freqHz = std::clamp (freqHz, 20.0f, sampleRate * 0.45f);
        const float w0 = 2.0f * float (M_PI) * freqHz / sampleRate;
        const float alpha = std::sin (w0) / (2.0f * std::max (q, 0.05f));
        const float cosw0 = std::cos (w0);
        const float a0 = 1.0f + alpha;
        b0 =  (1.0f + cosw0) * 0.5f / a0;
        b1 = -(1.0f + cosw0) / a0;
        b2 =  (1.0f + cosw0) * 0.5f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }

    void setLowpass (float freqHz, float q, float sampleRate)
    {
        freqHz = std::clamp (freqHz, 20.0f, sampleRate * 0.45f);
        const float w0 = 2.0f * float (M_PI) * freqHz / sampleRate;
        const float alpha = std::sin (w0) / (2.0f * std::max (q, 0.05f));
        const float cosw0 = std::cos (w0);
        const float a0 = 1.0f + alpha;
        b0 = (1.0f - cosw0) * 0.5f / a0;
        b1 = (1.0f - cosw0) / a0;
        b2 = (1.0f - cosw0) * 0.5f / a0;
        a1 = -2.0f * cosw0 / a0;
        a2 = (1.0f - alpha) / a0;
    }

    inline float process (float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = z2 = 0.0f; }
};

// Small fast xorshift PRNG -> white noise in [-1, 1].
struct NoiseGen
{
    uint32_t state = 0x9E3779B9u;
    inline float next()
    {
        state ^= state << 13; state ^= state >> 17; state ^= state << 5;
        return (float) (int32_t) state * (1.0f / 2147483648.0f);
    }
};

// PolyBLEP residual for band-limiting a discontinuity; t = phase in [0,1), dt = frequency / sampleRate.
inline float polyBlep (float t, float dt)
{
    if (t < dt)         { t /= dt;               return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt)  { t = (t - 1.0f) / dt;   return t * t + t + t + 1.0f; }
    return 0.0f;
}

inline float blepSquare (float phase, float dt)
{
    float v = phase < 0.5f ? 1.0f : -1.0f;
    v += polyBlep (phase, dt);
    float p2 = phase + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;
    v -= polyBlep (p2, dt);
    return v;
}

// ---------------------------------------------------------------------------
// Recipes. Built once; the plugin reads parameter ranges and defaults from here, so there is
// exactly one copy of every tuning range.
namespace detail
{
    inline Partial P (float ratio, float amp, float decay)
    { Partial p; p.ratio = ratio; p.amp = amp; p.decay = decay; p.noise = false; return p; }

    inline Partial N (float ratio, float amp, float decay, float q)
    { Partial p; p.ratio = ratio; p.amp = amp; p.decay = decay; p.noise = true; p.q = q; return p; }

    inline void tom (Recipe& r, float hz, float decayScale, float variance)
    {
        r.defaultHz = hz; r.minHz = hz * 0.5f; r.maxHz = hz * 2.2f;
        r.partials = { P(1.0f, 1.0f, 0.55f * decayScale), P(1.5f + variance, 0.45f, 0.30f * decayScale),
                       P(2.0f - variance, 0.28f, 0.20f * decayScale), P(2.6f, 0.12f, 0.14f * decayScale) };
        r.clickAmp = 0.28f; r.clickDecay = 0.007f;
        r.pitchEnvAmt = 0.5f; r.pitchEnvDecay = 0.035f;
    }

    inline std::array<Recipe, kNumInstruments> buildKitRecipes()
    {
        std::array<Recipe, kNumInstruments> rec{};

        // Bass drum: big steel shell, low inharmonic partials, pitch drop for punch
        {
            Recipe& r = rec[Bass];
            r.defaultHz = 55; r.minHz = 30; r.maxHz = 120;
            r.partials = { P(1.0f,1.0f,1.1f), P(1.50f,0.5f,0.6f), P(1.99f,0.32f,0.4f), P(2.55f,0.16f,0.28f) };
            r.clickAmp = 0.35f; r.clickDecay = 0.008f;
            r.pitchEnvAmt = 0.8f; r.pitchEnvDecay = 0.045f;
        }
        // Snare 1: tighter, metallic ring + bright snap
        {
            Recipe& r = rec[Snare1];
            r.defaultHz = 190; r.minHz = 100; r.maxHz = 400;
            r.partials = { P(1.0f,0.75f,0.16f), P(1.47f,0.4f,0.12f), P(2.03f,0.22f,0.09f),
                           N(8.0f,0.5f,0.16f,3.0f), N(16.0f,0.35f,0.10f,4.0f) };
            r.clickAmp = 0.5f; r.clickDecay = 0.006f;
        }
        // Snare 2: looser, buzzier, longer rattly noise tail (two close partials beat)
        {
            Recipe& r = rec[Snare2];
            r.defaultHz = 160; r.minHz = 90; r.maxHz = 350;
            r.partials = { P(1.0f,0.7f,0.22f), P(1.015f,0.5f,0.20f), P(1.9f,0.25f,0.14f),
                           N(6.0f,0.55f,0.30f,2.2f), N(11.0f,0.4f,0.22f,3.0f) };
            r.clickAmp = 0.4f; r.clickDecay = 0.007f;
        }
        // Toms: same shape, three registers, two slightly different drums per register
        tom (rec[TomLow1],   95, 1.05f,  0.00f);
        tom (rec[TomLow2],  105, 0.95f,  0.02f);
        tom (rec[TomMid1],  145, 1.00f,  0.00f);
        tom (rec[TomMid2],  160, 0.90f, -0.02f);
        tom (rec[TomHigh1], 210, 0.95f,  0.00f);
        tom (rec[TomHigh2], 230, 0.85f,  0.02f);

        // Splash: bright, short metallic burst
        {
            Recipe& r = rec[Splash];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 1000;
            r.metal = { 1.40f, 0.30f, 0.6f, 5000.0f, 0.5f, 12000.0f };
            r.clickAmp = 0.3f; r.clickDecay = 0.004f;
        }
        // Ride: long metallic wash + two sine "ping" partials
        {
            Recipe& r = rec[Ride];
            r.defaultHz = 420; r.minHz = 200; r.maxHz = 800;
            r.metal = { 0.90f, 1.6f, 0.7f, 3200.0f, 0.25f, 11000.0f };
            r.partials = { P(5.0f,0.035f,1.4f), P(7.4f,0.025f,0.9f) };
            r.clickAmp = 0.2f; r.clickDecay = 0.005f;
        }
        // Hi-hats: closed and open share a choke group
        {
            Recipe& r = rec[HihatClosed];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 900;
            r.metal = { 2.00f, 0.04f, 0.6f, 7000.0f, 0.35f, 12000.0f };
            r.clickAmp = 0.35f; r.clickDecay = 0.003f;
            r.chokeGroup = 1;
        }
        {
            Recipe& r = rec[HihatOpen];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 900;
            r.metal = { 1.50f, 0.6f, 0.6f, 6500.0f, 0.35f, 12000.0f };
            r.clickAmp = 0.3f; r.clickDecay = 0.004f;
            r.chokeGroup = 1;
        }
        // Cowbell: two close partials near a 1.48 ratio, plus upper partials
        {
            Recipe& r = rec[Cowbell];
            r.defaultHz = 560; r.minHz = 300; r.maxHz = 1100;
            r.partials = { P(1.0f,0.6f,0.35f), P(1.48f,0.6f,0.30f), P(3.0f,0.15f,0.15f), P(4.44f,0.15f,0.12f) };
            r.clickAmp = 0.25f; r.clickDecay = 0.004f;
        }
        // Rimshot: very short, high, click-dominant crack
        {
            Recipe& r = rec[Rimshot];
            r.defaultHz = 350; r.minHz = 180; r.maxHz = 700;
            r.partials = { P(1.0f,0.5f,0.045f), P(1.8f,0.35f,0.035f), N(6.0f,0.4f,0.05f,4.0f), N(12.0f,0.3f,0.03f,5.0f) };
            r.clickAmp = 0.7f; r.clickDecay = 0.0035f;
        }
        return rec;
    }
} // namespace detail

inline const std::array<Recipe, kNumInstruments>& kitRecipes()
{
    static const std::array<Recipe, kNumInstruments> table = detail::buildKitRecipes();
    return table;
}

// ---------------------------------------------------------------------------
// One active (ringing) hit of one instrument.
struct Voice
{
    bool active = false;
    float ageSamples = 0.0f;
    float hz = 200.0f;                 // tuning captured when the hit was struck
    float velocity = 1.0f;
    float phase[kMaxPartials] = {0};
    float envMul[kMaxPartials] = {0};  // current envelope value (starts at partial.amp)
    float envCoef[kMaxPartials] = {0}; // per-sample multiplicative decay
    Biquad bp[kMaxPartials];
    float mphase[kMetalOscs] = {0};    // metal bank oscillator phases in [0,1)
    Biquad hp1, hp2, lp1;              // cascaded high-pass and a low-pass (metal)
    float metalEnv = 0.0f;
    float metalCoef = 0.0f;
    float clickEnv = 0.0f;
    float clickCoef = 0.0f;
    bool  choking = false;             // fast fade-out triggered by another instrument
};

// ---------------------------------------------------------------------------
class DrumEngine
{
public:
    explicit DrumEngine (double sr = 44100.0) : recipes (kitRecipes()) { setSampleRate (sr); }

    void setSampleRate (double sr)
    {
        sampleRate = sr;
        for (int i = 0; i < kNumInstruments; ++i)
            currentHz[i] = recipes[(size_t) i].defaultHz;
    }

    const Recipe& getRecipe (int instrument) const { return recipes[(size_t) instrument]; }

    // Tuning for the NEXT trigger() of this instrument. Clamped to the recipe's range.
    // A note that is already ringing keeps the pitch it was struck at.
    void setPitchHz (int instrument, float hz)
    {
        const Recipe& r = recipes[(size_t) instrument];
        currentHz[instrument] = std::clamp (hz, r.minHz, r.maxHz);
    }

    float getPitchHz (int instrument) const { return currentHz[instrument]; }

    void trigger (int instrument, float velocity = 1.0f)
    {
        const Recipe& r = recipes[(size_t) instrument];

        // choke other instruments sharing this choke group (closed hat <-> open hat)
        if (r.chokeGroup != 0)
            for (int inst = 0; inst < kNumInstruments; ++inst)
                if (inst != instrument && recipes[(size_t) inst].chokeGroup == r.chokeGroup)
                    for (auto& v : voices[inst])
                        if (v.active) v.choking = true;

        // find a free voice slot, else steal the oldest
        Voice* slot = nullptr;
        float oldestAge = -1.0f;
        for (auto& v : voices[instrument])
        {
            if (! v.active) { slot = &v; break; }
            if (v.ageSamples > oldestAge) { oldestAge = v.ageSamples; slot = &v; }
        }

        slot->active = true;
        slot->ageSamples = 0.0f;
        slot->hz = currentHz[instrument];
        slot->velocity = std::clamp (velocity, 0.0f, 1.5f);
        slot->choking = false;
        slot->clickEnv = r.clickAmp * slot->velocity;
        slot->clickCoef = timeConst (r.clickDecay);

        for (int p = 0; p < kMaxPartials; ++p)
        {
            const Partial& pt = r.partials[(size_t) p];
            slot->phase[p] = 0.0f;
            slot->envMul[p] = pt.amp * slot->velocity;
            slot->envCoef[p] = timeConst (std::max (pt.decay, 0.001f));
            if (pt.noise)
            {
                slot->bp[p].reset();
                slot->bp[p].setBandpass (std::max (20.0f, slot->hz * pt.ratio), pt.q, (float) sampleRate);
            }
        }

        if (r.metal.amp > 0.0f)
        {
            for (int k = 0; k < kMetalOscs; ++k)
                slot->mphase[k] = 0.5f * (noise.next() + 1.0f);   // random start phases
            slot->hp1.reset(); slot->hp2.reset(); slot->lp1.reset();
            slot->hp1.setHighpass (r.metal.hpHz, 0.7071f, (float) sampleRate);
            slot->hp2.setHighpass (r.metal.hpHz, 0.7071f, (float) sampleRate);
            slot->lp1.setLowpass (r.metal.lpHz, 0.7071f, (float) sampleRate);
            slot->metalEnv = r.metal.amp * slot->velocity;
            slot->metalCoef = timeConst (std::max (r.metal.decay, 0.001f));
        }
        else
        {
            slot->metalEnv = 0.0f;
        }
    }

    void allNotesOff()
    {
        for (auto& bank : voices)
            for (auto& v : bank)
                v.active = false;
    }

    // Final output: renders numSamples of mono audio into 'out' (overwritten), through the master
    // gain and soft clip. |out[i]| < 1 always.
    void render (float* out, int numSamples)
    {
        std::fill (out, out + numSamples, 0.0f);
        renderRaw (out, numSamples);
        for (int n = 0; n < numSamples; ++n)
            out[n] = std::tanh (out[n] * kMasterGain);
    }

    // Voice sum BEFORE the master stage, ADDED into 'out' (caller clears). For tests and measurement.
    void renderRaw (float* out, int numSamples)
    {
        for (int inst = 0; inst < kNumInstruments; ++inst)
        {
            const Recipe& r = recipes[(size_t) inst];
            for (auto& v : voices[inst])
            {
                if (! v.active) continue;

                for (int n = 0; n < numSamples; ++n)
                {
                    float sample = 0.0f;
                    const float ageSec = v.ageSamples / (float) sampleRate;
                    const float pitchEnv = 1.0f + r.pitchEnvAmt * std::exp (-ageSec / std::max (r.pitchEnvDecay, 0.001f));

                    for (int p = 0; p < kMaxPartials; ++p)
                    {
                        const Partial& pt = r.partials[(size_t) p];
                        if (pt.amp <= 0.0f) continue;

                        if (pt.noise)
                        {
                            const float raw = noise.next();
                            sample += v.envMul[p] * v.bp[p].process (raw) * 3.0f; // compensate bandpass loss
                        }
                        else
                        {
                            const float freq = v.hz * pt.ratio * pitchEnv;
                            const float inc = 2.0f * float (M_PI) * freq / (float) sampleRate;
                            sample += v.envMul[p] * std::sin (v.phase[p]);
                            v.phase[p] += inc;
                            if (v.phase[p] > 2.0f * float (M_PI)) v.phase[p] -= 2.0f * float (M_PI);
                        }
                        v.envMul[p] *= v.choking ? 0.985f : v.envCoef[p];
                    }

                    if (r.metal.amp > 0.0f)
                    {
                        float bank = 0.0f;
                        for (int k = 0; k < kMetalOscs; ++k)
                        {
                            const float dt = v.hz * r.metal.baseScale * kMetalRatios[(size_t) k] / (float) sampleRate;
                            bank += blepSquare (v.mphase[k], dt);
                            v.mphase[k] += dt;
                            if (v.mphase[k] >= 1.0f) v.mphase[k] -= 1.0f;
                        }
                        const float src = bank * (1.0f / (float) kMetalOscs) + r.metal.noiseMix * noise.next();
                        sample += v.metalEnv * v.lp1.process (v.hp2.process (v.hp1.process (src)));
                        v.metalEnv *= v.choking ? 0.985f : v.metalCoef;
                    }

                    // click transient (broadband, gently shaped)
                    sample += v.clickEnv * noise.next();
                    v.clickEnv *= v.clickCoef;

                    out[n] += sample;
                    v.ageSamples += 1.0f;
                }

                // deactivate once inaudible
                bool audible = v.clickEnv > 0.0002f || (r.metal.amp > 0.0f && v.metalEnv > 0.0002f);
                for (int p = 0; p < kMaxPartials && ! audible; ++p)
                    if (r.partials[(size_t) p].amp > 0.0f && v.envMul[p] > 0.0002f) audible = true;
                if (! audible || v.ageSamples > (float) sampleRate * 8.0f)
                    v.active = false;
            }
        }
    }

private:
    double sampleRate = 44100.0;
    const std::array<Recipe, kNumInstruments>& recipes;
    float currentHz[kNumInstruments] = {0};
    Voice voices[kNumInstruments][kMaxInstances];
    NoiseGen noise;

    inline float timeConst (float seconds) const
    {
        // per-sample multiplier so that amplitude falls to 1/e after 'seconds'
        return std::exp (-1.0f / (seconds * (float) sampleRate));
    }
};

} // namespace oildrum
