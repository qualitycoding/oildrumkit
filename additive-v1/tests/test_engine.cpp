// test_engine.cpp: tests for DrumEngine.h and KitMap.h. No framework, no JUCE.
//   g++ -O2 -std=c++17 -Wall -I Source tests/test_engine.cpp -o build/test_engine && build/test_engine
// Exit status 0 = every check passed. Each section prints "k of N" so coverage is explicit.

#include "DrumEngine.h"
#include "KitMap.h"
#include <complex>
#include <vector>
#include <cstdio>
#include <string>

using namespace oildrum;

static int gChecks = 0, gFailed = 0;
#define CHECK(cond, ...) do { ++gChecks; if (!(cond)) { ++gFailed; std::printf("  FAIL %s:%d  %s  -> ", __FILE__, __LINE__, #cond); std::printf(__VA_ARGS__); std::printf("\n"); } } while (0)

static const double kSr = 44100.0;

// ---------------------------------------------------------------- helpers
static std::vector<float> renderOne (int inst, float hz, float vel, double seconds, bool raw, double sr = kSr)
{
    DrumEngine e (sr);
    e.setPitchHz (inst, hz);
    e.trigger (inst, vel);
    std::vector<float> out ((size_t) (seconds * sr));
    const int block = 256;
    for (size_t pos = 0; pos < out.size(); pos += block)
    {
        const int n = (int) std::min<size_t> (block, out.size() - pos);
        if (raw) { std::fill (out.begin() + (long) pos, out.begin() + (long) pos + n, 0.0f); e.renderRaw (out.data() + pos, n); }
        else e.render (out.data() + pos, n);
    }
    return out;
}

static void fft (std::vector<std::complex<double>>& a)
{
    const size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; ++i)
    {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap (a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1)
    {
        const double ang = -2.0 * M_PI / (double) len;
        const std::complex<double> wl (std::cos (ang), std::sin (ang));
        for (size_t i = 0; i < n; i += len)
        {
            std::complex<double> w (1.0);
            for (size_t k = 0; k < len / 2; ++k)
            {
                auto u = a[i + k], v = a[i + k + len / 2] * w;
                a[i + k] = u + v; a[i + k + len / 2] = u - v; w *= wl;
            }
        }
    }
}

// Power spectrum of x[start, start+len) with a Hann window, zero-padded to 2^17.
struct Spectrum { std::vector<double> p; double binHz; };
static Spectrum spectrum (const std::vector<float>& x, double startSec, double lenSec, double sr = kSr)
{
    const size_t N = 1u << 17;
    std::vector<std::complex<double>> a (N, 0.0);
    const size_t s = (size_t) (startSec * sr), L = std::min ((size_t) (lenSec * sr), x.size() - s);
    for (size_t i = 0; i < L; ++i) a[i] = x[s + i] * (0.5 - 0.5 * std::cos (2.0 * M_PI * (double) i / (double) (L - 1)));
    fft (a);
    Spectrum sp; sp.binHz = sr / (double) N; sp.p.resize (N / 2);
    for (size_t i = 0; i < N / 2; ++i) sp.p[i] = std::norm (a[i]);
    return sp;
}

static double dominantHz (const Spectrum& sp, double loHz, double hiHz)
{
    size_t best = (size_t) (loHz / sp.binHz); double bp = -1;
    for (size_t i = (size_t) (loHz / sp.binHz); i < (size_t) (hiHz / sp.binHz) && i < sp.p.size(); ++i)
        if (sp.p[i] > bp) { bp = sp.p[i]; best = i; }
    return (double) best * sp.binHz;
}

static double bandEnergy (const Spectrum& sp, double loHz, double hiHz)
{
    double e = 0;
    for (size_t i = (size_t) (loHz / sp.binHz); i < (size_t) (hiHz / sp.binHz) && i < sp.p.size(); ++i) e += sp.p[i];
    return e;
}

static double f95 (const Spectrum& sp)
{
    const double total = bandEnergy (sp, 100.0, 20000.0); double acc = 0;
    for (size_t i = (size_t) (100.0 / sp.binHz); i < sp.p.size(); ++i)
    { acc += sp.p[i]; if (acc >= 0.95 * total) return (double) i * sp.binHz; }
    return 20000.0;
}

static float peakOf (const std::vector<float>& x) { float p = 0; for (float v : x) p = std::max (p, std::fabs (v)); return p; }
static double rmsOf (const std::vector<float>& x, double s0, double s1, double sr = kSr)
{
    double acc = 0; size_t a = (size_t) (s0 * sr), b = std::min (x.size(), (size_t) (s1 * sr));
    for (size_t i = a; i < b; ++i) acc += (double) x[i] * x[i];
    return std::sqrt (acc / (double) (b - a));
}
static bool allFinite (const std::vector<float>& x) { for (float v : x) if (! std::isfinite (v)) return false; return true; }

// ---------------------------------------------------------------- sections
static void testTable()
{
    std::printf ("[1] kit table and MIDI map\n");
    // Independent copy of the General MIDI channel-10 notes (K-20261008T173540Z), one per instrument.
    struct Gm { int note; int inst; } gm[] = {
        {36,Bass},{37,Rimshot},{38,Snare1},{40,Snare2},{41,TomLow1},{43,TomLow2},{45,TomMid1},{47,TomMid2},
        {48,TomHigh1},{50,TomHigh2},{42,HihatClosed},{46,HihatOpen},{51,Ride},{55,Splash},{56,Cowbell} };
    int ok = 0;
    for (auto& g : gm) { const bool good = midiNoteToInstrument (g.note) == g.inst; ok += good; CHECK (good, "note %d", g.note); }
    std::printf ("  notes matching the General MIDI list: %d of %d\n", ok, (int) (sizeof (gm) / sizeof (gm[0])));
    CHECK (midiNoteToInstrument (39) == -1 && midiNoteToInstrument (0) == -1 && midiNoteToInstrument (127) == -1, "unmapped notes must return -1");
    int ranges = 0;
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const Recipe& r = kitRecipes()[(size_t) i];
        const bool good = r.minHz > 0 && r.minHz < r.defaultHz && r.defaultHz < r.maxHz;
        ranges += good; CHECK (good, "range of %s", instrumentName (i));
    }
    std::printf ("  instruments with min < default < max: %d of %d\n", ranges, kNumInstruments);
}

static void testAllVoices()
{
    std::printf ("[2] all %d voices: finite, audible, bounded, silent at the end\n", kNumInstruments);
    int finite = 0, audible = 0, bounded = 0, silent = 0;
    for (int i = 0; i < kNumInstruments; ++i)
    {
        auto raw = renderOne (i, kitRecipes()[(size_t) i].defaultHz, 1.0f, 10.0, true);
        auto out = renderOne (i, kitRecipes()[(size_t) i].defaultHz, 1.0f, 10.0, false);
        const bool f = allFinite (raw) && allFinite (out);
        const bool a = peakOf (out) > 0.02f;
        const bool b = peakOf (out) < 1.0f;
        const bool s = rmsOf (out, 9.0, 10.0) < 1e-3;
        finite += f; audible += a; bounded += b; silent += s;
        CHECK (f, "%s not finite", instrumentName (i)); CHECK (a, "%s too quiet: %.4f", instrumentName (i), peakOf (out));
        CHECK (b, "%s peak %.4f", instrumentName (i), peakOf (out)); CHECK (s, "%s still ringing at 9-10 s", instrumentName (i));
        std::printf ("  %-14s raw peak %.3f  output peak %.3f\n", instrumentName (i), peakOf (raw), peakOf (out));
    }
    std::printf ("  finite %d of %d, audible %d of %d, bounded below 1.0 %d of %d, silent by 9 s %d of %d\n",
                 finite, kNumInstruments, audible, kNumInstruments, bounded, kNumInstruments, silent, kNumInstruments);
}

static void testGainStage()
{
    std::printf ("[3] gain stage through the shipped path (render, not renderRaw)\n");
    auto hit = [&] (std::vector<int> insts, bool raw)
    {
        DrumEngine e (kSr); for (int i : insts) e.trigger (i, 1.0f);
        std::vector<float> out ((size_t) kSr * 2, 0.0f);
        for (size_t pos = 0; pos < out.size(); pos += 256) { const int n = (int) std::min<size_t> (256, out.size() - pos);
            if (raw) e.renderRaw (out.data() + pos, n); else e.render (out.data() + pos, n); }
        return peakOf (out);
    };
    const float raw4 = hit ({ Bass, Snare1, Ride, TomLow1 }, true), out4 = hit ({ Bass, Snare1, Ride, TomLow1 }, false);
    std::printf ("  four-voice hit: raw peak %.3f, output peak %.3f\n", raw4, out4);
    CHECK (raw4 > 1.0f, "premise: raw sum should exceed 1.0 (%.3f)", raw4);
    CHECK (out4 <= 1.0f, "output peak %.3f", out4);
    std::vector<int> all; for (int i = 0; i < kNumInstruments; ++i) all.push_back (i);
    const float outAll = hit (all, false);
    std::printf ("  all 15 voices at once: output peak %.3f\n", outAll);
    CHECK (outAll <= 1.0f, "output peak %.3f", outAll);
    // single-hit level should survive the master stage at a usable level
    auto bass = renderOne (Bass, 55, 1.0f, 1.0, false);
    CHECK (peakOf (bass) > 0.25f, "bass output peak %.3f is too quiet", peakOf (bass));
}

static void testTuning()
{
    std::printf ("[4] tuning follows the pitch parameter (strongest partial / tuning stays constant)\n");
    const int insts[] = { Bass, Snare1, Snare2, TomLow1, TomMid2, TomHigh2, Cowbell };
    int ok = 0;
    for (int inst : insts)
    {
        const Recipe& r = kitRecipes()[(size_t) inst];
        double ratios[3]; const float hz[3] = { r.minHz * 1.05f, r.defaultHz, r.maxHz * 0.95f };
        for (int k = 0; k < 3; ++k)
        {
            auto x = renderOne (inst, hz[k], 1.0f, 0.8, true);
            auto sp = spectrum (x, 0.2, 0.3);
            ratios[k] = dominantHz (sp, 0.5 * hz[k], 2.6 * hz[k]) / hz[k];
        }
        const bool good = std::fabs (ratios[0] / ratios[1] - 1) < 0.03 && std::fabs (ratios[2] / ratios[1] - 1) < 0.03;
        ok += good;
        CHECK (good, "%s ratios %.3f %.3f %.3f", instrumentName (inst), ratios[0], ratios[1], ratios[2]);
        std::printf ("  %-10s strongest partial / tuning at min, default, max: %.3f %.3f %.3f\n", instrumentName (inst), ratios[0], ratios[1], ratios[2]);
    }
    std::printf ("  voices whose partials scale with tuning: %d of %d\n", ok, (int) (sizeof (insts) / sizeof (insts[0])));

    // sample-rate independence: the same tuning gives the same pitch at 48 kHz and 96 kHz
    int srOk = 0;
    for (double sr : { 48000.0, 96000.0 })
    {
        auto x = renderOne (Bass, 55.0f, 1.0f, 0.8, true, sr);
        auto sp = spectrum (x, 0.2, 0.3, sr);
        const double f = dominantHz (sp, 30, 120);
        const bool good = std::fabs (f / 55.0 - 1) < 0.03; srOk += good;
        CHECK (good, "bass at %.0f Hz sample rate: %.2f Hz", sr, f);
    }
    std::printf ("  bass pitch correct at 48 kHz and 96 kHz: %d of 2\n", srOk);
}

static void testPitchAtNoteOn()
{
    std::printf ("[5] pitch is taken at note-on: a ringing note does not follow later changes\n");
    DrumEngine e (kSr);
    e.setPitchHz (Cowbell, 560); e.trigger (Cowbell, 1.0f);
    std::vector<float> a ((size_t) (0.2 * kSr)), b ((size_t) (0.6 * kSr));
    e.renderRaw (a.data(), (int) a.size());
    e.setPitchHz (Cowbell, 1100);
    e.renderRaw (b.data(), (int) b.size());
    auto spRing = spectrum (b, 0.05, 0.2);
    const double fRing = dominantHz (spRing, 300, 2500);
    CHECK (std::fabs (fRing / 560.0 - 1) < 0.03 || std::fabs (fRing / (560.0 * 1.48) - 1) < 0.03, "ringing note moved to %.1f Hz", fRing);
    std::printf ("  ringing cowbell after retune to 1100 Hz: strongest partial %.1f Hz (stays at its 560 Hz family)\n", fRing);
    // the NEXT hit uses the new pitch
    std::vector<float> c ((size_t) (0.5 * kSr), 0.0f);
    DrumEngine e2 (kSr); e2.setPitchHz (Cowbell, 1100); e2.trigger (Cowbell, 1.0f); e2.renderRaw (c.data(), (int) c.size());
    const double fNext = dominantHz (spectrum (c, 0.05, 0.2), 300, 2500);
    CHECK (std::fabs (fNext / 1100.0 - 1) < 0.03 || std::fabs (fNext / (1100.0 * 1.48) - 1) < 0.03, "next hit at %.1f Hz", fNext);
    std::printf ("  next hit at 1100 Hz tuning: strongest partial %.1f Hz\n", fNext);
}

static void testVelocityAndChoke()
{
    std::printf ("[6] velocity scaling (all voices) and hi-hat choke\n");
    int ok = 0;
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const float hz = kitRecipes()[(size_t) i].defaultHz;
        auto full = renderOne (i, hz, 1.0f, 1.0, true), half = renderOne (i, hz, 0.5f, 1.0, true);
        const double ratio = rmsOf (half, 0, 1.0) / rmsOf (full, 0, 1.0);
        const bool good = std::fabs (ratio - 0.5) < 0.01; ok += good;
        CHECK (good, "%s velocity 0.5 gave %.4f of full level", instrumentName (i), ratio);
    }
    std::printf ("  velocity 0.5 gives half the level: %d of %d\n", ok, kNumInstruments);

    auto openAlone = renderOne (HihatOpen, 500, 1.0f, 0.6, false);
    DrumEngine e (kSr); e.trigger (HihatOpen, 1.0f);
    std::vector<float> choked ((size_t) (0.6 * kSr), 0.0f);
    for (size_t pos = 0; pos < choked.size(); pos += 256)
    {
        if (pos >= (size_t) (0.2 * kSr) && pos < (size_t) (0.2 * kSr) + 256) e.trigger (HihatClosed, 0.0f); // silent closed hit chokes the open hat
        const int n = (int) std::min<size_t> (256, choked.size() - pos); e.render (choked.data() + pos, n);
    }
    const double a = rmsOf (openAlone, 0.35, 0.45), c = rmsOf (choked, 0.35, 0.45);
    std::printf ("  open hat level 0.35-0.45 s: alone %.5f, after a closed-hat strike at 0.2 s %.6f\n", a, c);
    CHECK (c < 0.05 * a, "open hat not choked: %.6f vs %.6f", c, a);
}

static void testCymbalBrightness()
{
    std::printf ("[7] cymbals and hats: energy distribution (first 0.5 s, raw voice)\n");
    // DESIGN TARGET, not a measured property of real cymbals (UNVERIFIED): at least 80% of the
    // energy above 2 kHz. The v1 recipes had 0.6-12%.
    const double target = 0.80;
    const int insts[] = { Splash, Ride, HihatClosed, HihatOpen };
    int ok = 0, total = 0;
    for (int inst : insts)
    {
        const Recipe& r = kitRecipes()[(size_t) inst];
        for (float hz : { r.minHz * 1.05f, r.defaultHz, r.maxHz * 0.95f })
        {
            auto x = renderOne (inst, hz, 1.0f, 0.6, true);
            auto sp = spectrum (x, 0.0, 0.5);
            const double frac = bandEnergy (sp, 2000, 20000) / bandEnergy (sp, 100, 20000);
            const double hi = bandEnergy (sp, 16000, 20000) / bandEnergy (sp, 100, 20000);
            const bool good = frac >= target; ok += good; ++total;
            CHECK (good, "%s at %.0f Hz tuning: %.1f%% above 2 kHz", instrumentName (inst), hz, 100 * frac);
            std::printf ("  %-14s tuning %5.0f Hz: above 2 kHz %5.1f%%, above 16 kHz %4.1f%%, f95 %6.0f Hz\n", instrumentName (inst), hz, 100 * frac, 100 * hi, f95 (sp));
        }
    }
    std::printf ("  cases with at least 80%% of energy above 2 kHz: %d of %d\n", ok, total);
}

int main()
{
    testTable();
    testAllVoices();
    testGainStage();
    testTuning();
    testPitchAtNoteOn();
    testVelocityAndChoke();
    testCymbalBrightness();
    std::printf ("\n%d checks, %d failed\n", gChecks, gFailed);
    return gFailed == 0 ? 0 : 1;
}
