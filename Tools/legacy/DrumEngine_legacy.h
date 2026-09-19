// DrumEngine.h
//
// Portable, header-only modal-synthesis engine for an "oil drum" kit.
// No external dependencies (no JUCE, no STL beyond <cmath>/<array>/<cstdint>),
// so this exact file is used both by the standalone WAV demo renderer and by
// the JUCE VST3 plugin (Source/PluginProcessor.cpp #includes it directly).
//
// SOUND MODEL
// Every drum is modeled as a small bank of "partials". Each partial is either
//   - a sine oscillator (pitched content: shell resonances), or
//   - a resonant bandpassed noise stream (metallic/rattly/hiss content).
// Each partial has its own amplitude and exponential decay time, and its
// frequency is expressed as a ratio of the instrument's *tunable* fundamental
// (the "pitch" parameter). Because the ratios are deliberately inharmonic
// (not small integer multiples), the result reads as struck sheet steel
// rather than a clean harmonic drum membrane -- i.e. an oil drum, not a tom.
//
// A short broadband noise "click" partial is layered under every hit for
// stick/mallet attack transient, and toms/bass additionally sweep their sine
// partials' pitch downward for the first ~30ms for punch (classic drum-synth
// pitch-envelope trick).
//
// Everything is sample-accurate and allocation-free after construction, so
// it is safe to call process() from a real-time audio thread.

#pragma once
#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>

namespace oildrum
{

constexpr float kPi = 3.14159265358979323846f;
constexpr int kMaxPartials = 8;
constexpr int kMaxInstances = 4;   // overlapping hits per instrument (for rolls/flams)
constexpr int kNumInstruments = 15;

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

enum HammerType {
    MetalHammer = 0,
    WoodStick,
    RubberMallet
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

// ---------------------------------------------------------------------------
// One resonant partial in a drum's recipe.
struct Partial
{
    float ratio  = 1.0f;   // frequency = fundamentalHz * ratio
    float amp    = 0.0f;   // 0..~1
    float decay  = 0.2f;   // seconds, exponential time constant
    bool  noise  = false;  // false = sine oscillator, true = resonant bandpass noise
    float q      = 10.0f;  // resonance (bandpass Q), only used when noise == true
};

// A full instrument recipe.
struct Recipe
{
    float defaultHz = 200.0f;
    float minHz     = 40.0f;
    float maxHz     = 800.0f;
    std::array<Partial, kMaxPartials> partials{};
    float clickAmp     = 0.15f;   // broadband attack transient
    float clickDecay   = 0.006f;  // seconds
    float pitchEnvAmt  = 0.0f;    // extra frequency multiplier at t=0 (sine partials only)
    float pitchEnvDecay= 0.03f;   // seconds
    int   chokeGroup   = 0;       // instruments sharing a nonzero group choke each other
};

// ---------------------------------------------------------------------------
// Minimal RBJ biquad bandpass filter (constant skirt gain, peak = Q).
struct Biquad
{
    float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1 = 0, z2 = 0;

    void setBandpass (float freqHz, float q, float sampleRate)
    {
        freqHz = std::clamp (freqHz, 20.0f, sampleRate * 0.45f);
        const float w0 = 2.0f * kPi * freqHz / sampleRate;
        const float alpha = std::sin (w0) / (2.0f * std::max (q, 0.05f));
        const float cosw0 = std::cos (w0);

        const float a0 =  1.0f + alpha;
        b0 =  alpha            / a0;
        b1 =  0.0f              / a0;
        b2 = -alpha            / a0;
        a1 = -2.0f * cosw0      / a0;
        a2 =  (1.0f - alpha)    / a0;
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

// ---------------------------------------------------------------------------
// One active (ringing) hit of one instrument.
struct Voice
{
    bool active = false;
    float ageSamples = 0.0f;
    float hz = 200.0f;                 // fundamental captured/live for this hit
    float velocity = 1.0f;
    float phase[kMaxPartials] = {0};
    float envMul[kMaxPartials] = {0};  // current envelope value (starts at partial.amp)
    float envCoef[kMaxPartials] = {0}; // per-sample multiplicative decay
    Biquad bp[kMaxPartials];
    float clickEnv = 0.0f;
    float clickCoef = 0.0f;
    float clickLpState = 0.0f;
    float clickLpAlpha = 1.0f; // 1.0 = no filtering
    bool  choking = false;             // fast fade-out triggered by another instrument
};

// ---------------------------------------------------------------------------
class DrumEngine
{
public:
    explicit DrumEngine (double sampleRate = 44100.0) { setSampleRate (sampleRate); }

    void setHammer(HammerType type) { currentHammer = type; }

    void setSampleRate (double sr)
    {
        sampleRate = sr;
        buildDefaultRecipes();
        for (int i = 0; i < kNumInstruments; ++i) {
            targetHz[i] = recipes[i].defaultHz;
            currentHz[i] = recipes[i].defaultHz;
        }
    }

    const Recipe& getRecipe (int instrument) const { return recipes[instrument]; }

    // "Pitch" parameter: set the live fundamental (Hz) for an instrument.
    // Affects the next trigger, and (deliberately) continues to affect any
    // currently-ringing voice too, so twisting the knob while a note rings
    // gives an audible pitch-glide -- handy for tuning by ear.
    void setPitchHz (int instrument, float hz)
    {
        const Recipe& r = recipes[instrument];
        targetHz[instrument] = std::clamp (hz, r.minHz, r.maxHz);
    }

    float getPitchHz (int instrument) const { return currentHz[instrument]; }

    void trigger (int instrument, float velocity = 1.0f)
    {
        const Recipe& r = recipes[instrument];

        if (r.chokeGroup != 0)
            for (int inst = 0; inst < kNumInstruments; ++inst)
                if (inst != instrument && recipes[inst].chokeGroup == r.chokeGroup)
                    for (auto& v : voices[inst])
                        if (v.active) v.choking = true;

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

        float hammerAmpMul = 1.0f;
        float hammerDecayMul = 1.0f;
        float lpAlpha = 1.0f;

        switch (currentHammer)
        {
            case MetalHammer:
                hammerAmpMul = 1.5f;
                hammerDecayMul = 0.4f;
                lpAlpha = 1.0f;
                break;
            case WoodStick:
                hammerAmpMul = 1.0f;
                hammerDecayMul = 1.0f;
                lpAlpha = 0.6f;
                break;
            case RubberMallet:
                hammerAmpMul = 0.6f;
                hammerDecayMul = 2.5f;
                lpAlpha = 0.15f;
                break;
        }

        const float randomizedAmp = r.clickAmp * hammerAmpMul * (1.0f + 0.15f * noise.next());
        const float randomizedDecay = r.clickDecay * hammerDecayMul * (1.0f + 0.10f * noise.next());

        slot->clickEnv = randomizedAmp * slot->velocity;
        slot->clickCoef = timeConst (std::max(randomizedDecay, 0.001f));
        slot->clickLpAlpha = lpAlpha;
        slot->clickLpState = 0.0f; 

        for (int p = 0; p < kMaxPartials; ++p)
        {
            const Partial& pt = r.partials[p];
            slot->phase[p] = 0.0f;
            slot->envMul[p] = pt.amp * slot->velocity;
            slot->envCoef[p] = timeConst (std::max (pt.decay, 0.001f));
            if (pt.noise)
            {
                slot->bp[p].reset();
                slot->bp[p].setBandpass (std::max (20.0f, slot->hz * pt.ratio), pt.q, (float) sampleRate);
            }
        }
    }

    void allNotesOff()
    {
        for (auto& bank : voices)
            for (auto& v : bank)
                v.active = false;
    }

    // Renders numSamples of mono audio, ADDING into 'out' (caller should clear first).
    void process (float* out, int numSamples)
    {
        // Calculate a smoothing coefficient for roughly ~20ms response time
        const float smoothCoef = std::exp (-1.0f / (0.02f * (float)sampleRate));

        for (int inst = 0; inst < kNumInstruments; ++inst)
        {
            const Recipe& r = recipes[inst];

            // Smooth the frequency parameter toward the target (UI knob value)
            currentHz[inst] = targetHz[inst] + smoothCoef * (currentHz[inst] - targetHz[inst]);

            for (auto& v : voices[inst])
            {
                if (! v.active) continue;

                // Live-update the ringing voice frequency so we get a smooth pitch glide
                v.hz = currentHz[inst];

                for (int n = 0; n < numSamples; ++n)
                {
                    float sample = 0.0f;
                    const float ageSec = v.ageSamples / (float) sampleRate;
                    const float pitchEnv = 1.0f + r.pitchEnvAmt * std::exp (-ageSec / std::max (r.pitchEnvDecay, 0.001f));

                    for (int p = 0; p < kMaxPartials; ++p)
                    {
                        const Partial& pt = r.partials[p];
                        if (pt.amp <= 0.0f) continue;

                        if (pt.noise)
                        {
                            const float raw = noise.next();
                            sample += v.envMul[p] * v.bp[p].process (raw) * 3.0f; // compensate bandpass loss
                        }
                        else
                        {
                            const float freq = v.hz * pt.ratio * pitchEnv;
                            const float inc = 2.0f * kPi * freq / (float) sampleRate;
                            sample += v.envMul[p] * std::sin (v.phase[p]);
                            v.phase[p] += inc;
                            if (v.phase[p] > 2.0f * kPi) v.phase[p] -= 2.0f * kPi;
                        }
                        v.envMul[p] *= v.choking ? 0.985f : pt.decay > 0 ? v.envCoef[p] : 1.0f;
                    }

                    // --- NEW: Hammer transient (one-pole lowpass filtered noise) ---
                    const float rawClick = noise.next();
                    v.clickLpState += v.clickLpAlpha * (rawClick - v.clickLpState);
                    sample += v.clickEnv * v.clickLpState;
                    v.clickEnv *= v.clickCoef;

                    out[n] += sample;
                    v.ageSamples += 1.0f;
                }

                // deactivate once inaudible
                bool audible = v.clickEnv > 0.0002f;
                for (int p = 0; p < kMaxPartials && ! audible; ++p)
                    if (r.partials[p].amp > 0.0f && v.envMul[p] > 0.0002f) audible = true;
                if (! audible || v.ageSamples > (float) sampleRate * 8.0f)
                    v.active = false;
            }
        }
    }

private:
    double sampleRate = 44100.0;
    Recipe recipes[kNumInstruments];
    float targetHz[kNumInstruments] = {0};   // ADD THIS: Where the knob is currently set
    float currentHz[kNumInstruments] = {0};  // THIS NOW BECOMES: The smoothed live value
    Voice voices[kNumInstruments][kMaxInstances];
    NoiseGen noise;
    HammerType currentHammer = WoodStick;

    inline float timeConst (float seconds) const
    {
        // per-sample multiplier so that amplitude falls to 1/e after 'seconds'
        return std::exp (-1.0f / (seconds * (float) sampleRate));
    }

    static Partial P (float ratio, float amp, float decay)
    { Partial p; p.ratio = ratio; p.amp = amp; p.decay = decay; p.noise = false; return p; }

    static Partial N (float ratio, float amp, float decay, float q)
    { Partial p; p.ratio = ratio; p.amp = amp; p.decay = decay; p.noise = true; p.q = q; return p; }

    void buildDefaultRecipes()
    {
        // ---- Bass drum: big steel shell, low inharmonic partials, pitch drop punch
        {
            Recipe& r = recipes[Bass];
            r.defaultHz = 55; r.minHz = 30; r.maxHz = 120;
            r.partials = { P(1.0f,1.0f,1.1f), P(1.50f,0.5f,0.6f), P(1.99f,0.32f,0.4f), P(2.55f,0.16f,0.28f) };
            r.clickAmp = 0.35f; r.clickDecay = 0.008f;
            r.pitchEnvAmt = 0.8f; r.pitchEnvDecay = 0.045f;
        }
        // ---- Snare 1: tighter, metallic ring + bright snap
        {
            Recipe& r = recipes[Snare1];
            r.defaultHz = 190; r.minHz = 100; r.maxHz = 400;
            r.partials = { P(1.0f,0.75f,0.16f), P(1.47f,0.4f,0.12f), P(2.03f,0.22f,0.09f),
                           N(8.0f,0.5f,0.16f,3.0f), N(16.0f,0.35f,0.10f,4.0f) };
            r.clickAmp = 0.5f; r.clickDecay = 0.006f;
        }
        // ---- Snare 2: looser, buzzier, longer rattly noise tail (two close partials beat)
        {
            Recipe& r = recipes[Snare2];
            r.defaultHz = 160; r.minHz = 90; r.maxHz = 350;
            r.partials = { P(1.0f,0.7f,0.22f), P(1.015f,0.5f,0.20f), P(1.9f,0.25f,0.14f),
                           N(6.0f,0.55f,0.30f,2.2f), N(11.0f,0.4f,0.22f,3.0f) };
            r.clickAmp = 0.4f; r.clickDecay = 0.007f;
        }
        // ---- Toms (low/mid/high, two of each). Same shape, different register + slight
        //      variation between the "1" and "2" of each pair for a natural double-drum feel.
        auto tom = [] (Recipe& r, float hz, float decayScale, float variance)
        {
            r.defaultHz = hz; r.minHz = hz * 0.5f; r.maxHz = hz * 2.2f;
            r.partials = { P(1.0f,1.0f,0.55f*decayScale), P(1.5f+variance,0.45f,0.30f*decayScale),
                           P(2.0f-variance,0.28f,0.20f*decayScale), P(2.6f,0.12f,0.14f*decayScale) };
            r.clickAmp = 0.28f; r.clickDecay = 0.007f;
            r.pitchEnvAmt = 0.5f; r.pitchEnvDecay = 0.035f;
        };
        tom (recipes[TomLow1],  95,  1.05f,  0.00f);
        tom (recipes[TomLow2], 105,  0.95f,  0.02f);
        tom (recipes[TomMid1], 145,  1.00f,  0.00f);
        tom (recipes[TomMid2], 160,  0.90f, -0.02f);
        tom (recipes[TomHigh1],210,  0.95f,  0.00f);
        tom (recipes[TomHigh2],230,  0.85f,  0.02f);

        // ---- Splash: bright, short, dense inharmonic noise-partial shimmer
        {
            Recipe& r = recipes[Splash];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 1000;
            r.partials = { N(1.0f,0.5f,0.30f,6.0f), N(1.8f,0.45f,0.26f,7.0f), N(2.7f,0.4f,0.22f,8.0f),
                           N(3.9f,0.3f,0.18f,9.0f), N(5.6f,0.22f,0.14f,10.0f), P(1.0f,0.15f,0.25f) };
            r.clickAmp = 0.3f; r.clickDecay = 0.004f;
        }
        // ---- Ride: sustained shimmer + a sine "ping" on top
        {
            Recipe& r = recipes[Ride];
            r.defaultHz = 420; r.minHz = 200; r.maxHz = 800;
            r.partials = { N(1.0f,0.35f,2.2f,7.0f), N(1.9f,0.30f,1.8f,8.0f), N(2.8f,0.25f,1.4f,9.0f),
                           N(4.2f,0.18f,1.0f,10.0f), P(1.0f,0.25f,1.6f), P(2.4f,0.12f,0.9f) };
            r.clickAmp = 0.2f; r.clickDecay = 0.005f;
        }
        // ---- Hi-hats: closed/open share a choke group and the same noise-bank character
        {
            Recipe& r = recipes[HihatClosed];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 900;
            r.partials = { N(1.0f,0.5f,0.05f,5.0f), N(1.8f,0.45f,0.045f,6.0f), N(2.6f,0.4f,0.04f,7.0f),
                           N(3.7f,0.3f,0.035f,8.0f), N(5.3f,0.2f,0.03f,9.0f) };
            r.clickAmp = 0.35f; r.clickDecay = 0.003f;
            r.chokeGroup = 1;
        }
        {
            Recipe& r = recipes[HihatOpen];
            r.defaultHz = 500; r.minHz = 250; r.maxHz = 900;
            r.partials = { N(1.0f,0.45f,0.9f,5.0f), N(1.8f,0.4f,0.75f,6.0f), N(2.6f,0.35f,0.6f,7.0f),
                           N(3.7f,0.25f,0.45f,8.0f), N(5.3f,0.18f,0.35f,9.0f) };
            r.clickAmp = 0.3f; r.clickDecay = 0.004f;
            r.chokeGroup = 1;
        }
        // ---- Cowbell: classic two close square-ish partials (odd harmonics) at ~1.48 ratio
        {
            Recipe& r = recipes[Cowbell];
            r.defaultHz = 560; r.minHz = 300; r.maxHz = 1100;
            r.partials = { P(1.0f,0.6f,0.35f), P(1.48f,0.6f,0.30f), P(3.0f,0.15f,0.15f), P(4.44f,0.15f,0.12f) };
            r.clickAmp = 0.25f; r.clickDecay = 0.004f;
        }
        // ---- Rimshot: very short, high, sharp click-dominant crack
        {
            Recipe& r = recipes[Rimshot];
            r.defaultHz = 350; r.minHz = 180; r.maxHz = 700;
            r.partials = { P(1.0f,0.5f,0.045f), P(1.8f,0.35f,0.035f), N(6.0f,0.4f,0.05f,4.0f), N(12.0f,0.3f,0.03f,5.0f) };
            r.clickAmp = 0.7f; r.clickDecay = 0.0035f;
        }
    }
};

} // namespace oildrum
