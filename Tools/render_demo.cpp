// render_demo.cpp -- deterministic offline renderer for A/B testing the drum engine.
//
// Build against ANY engine header with -DENGINE_HEADER='"path/to/DrumEngine.h"'.
// Default is ../Source/DrumEngine.h. An engine that defines OILDRUM_HAS_STEREO gets
// stereo output via process(L,R,n); otherwise the mono process(out,n) is used and
// the WAV is written as mono.
//
//   render_demo demo    OUT_PREFIX [ROOM_MIX]  -> OUT_PREFIX.wav, _A.._D.wav, .events.csv, .meta.json
//   render_demo metrics OUT_DIR         -> isolated single hits, OUT_DIR/iXX_vYY.wav (3 s each)
//
// ROOM_MIX (0..0.4, default 0.12) only affects engines that define setRoomMix.
#ifndef ENGINE_HEADER
#define ENGINE_HEADER "../Source/DrumEngine.h"
#endif
#include ENGINE_HEADER
#include "wav.h"
#if defined(__SSE__)
#include <xmmintrin.h>
#endif
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <memory>

using namespace oildrum;

static const int    kSR    = 48000;
static const int    kBlock = 512;      // mimic a typical host buffer size
#ifdef OILDRUM_HAS_STEREO
static const int    kCh = 2;
#else
static const int    kCh = 1;
#endif
static float gRoom = 0.12f;      // demo-mode room mix (engines with setRoomMix); 3rd CLI arg overrides

struct Event { double t; int inst; float vel; char section; };

static void setupEngine (DrumEngine& e)
{
    e.setHammer (WoodStick);                 // plugin default
#ifdef OILDRUM_HAS_STEREO
    e.setRoomMix (gRoom);
#endif
}

// Renders events (sorted by time) into L/R buffers of totalSamples length.
static void renderEvents (DrumEngine& e, std::vector<Event> ev, int totalSamples,
                          std::vector<float>& L, std::vector<float>& R)
{
    std::sort (ev.begin(), ev.end(), [] (const Event& a, const Event& b) { return a.t < b.t; });
    L.assign ((size_t) totalSamples, 0.0f);
    R.assign ((size_t) totalSamples, 0.0f);
    size_t next = 0;
    int pos = 0;
    while (pos < totalSamples)
    {
        int blockEnd = std::min (pos + kBlock, totalSamples);
        int cur = pos;
        while (true)
        {
            int evSample = blockEnd;
            if (next < ev.size())
                evSample = std::min (blockEnd, (int) std::llround (ev[next].t * kSR));
            if (evSample > cur)
            {
#ifdef OILDRUM_HAS_STEREO
                e.process (L.data() + cur, R.data() + cur, evSample - cur);
#else
                e.process (L.data() + cur, evSample - cur);
#endif
                cur = evSample;
            }
            if (next < ev.size() && (int) std::llround (ev[next].t * kSR) <= cur)
            {
                e.trigger (ev[next].inst, ev[next].vel);
                ++next;
                continue;
            }
            if (cur >= blockEnd) break;
        }
        pos = blockEnd;
    }
#ifndef OILDRUM_HAS_STEREO
    R = L;
#endif
}

static bool checkFinite (const std::vector<float>& v, float& peak)
{
    peak = 0.0f;
    for (float x : v) { if (! std::isfinite (x)) return false; peak = std::max (peak, std::fabs (x)); }
    return true;
}

static void addGroove (std::vector<Event>& ev, double t0)
{
    const double beat = 60.0 / 96.0;
    for (int bar = 0; bar < 8; ++bar)
    {
        const double b0 = t0 + bar * 4 * beat;
        // hats: 8ths, accented on the beat
        for (int i = 0; i < 8; ++i)
        {
            const bool open = (bar == 3 && i == 7);
            ev.push_back ({ b0 + i * beat * 0.5, open ? HihatOpen : HihatClosed, (i % 2 == 0) ? 0.8f : 0.5f, 'C' });
        }
        ev.push_back ({ b0 + 0 * beat, Bass, 1.0f, 'C' });
        ev.push_back ({ b0 + 2 * beat, Bass, 0.9f, 'C' });
        if (bar % 2 == 1) ev.push_back ({ b0 + 3.5 * beat, Bass, 0.7f, 'C' });
        ev.push_back ({ b0 + 1 * beat, Snare1, 0.95f, 'C' });
        ev.push_back ({ b0 + 3 * beat, Snare1, 0.95f, 'C' });
        if (bar == 0) ev.push_back ({ b0, Splash, 0.9f, 'C' });
        if (bar == 4) ev.push_back ({ b0, Ride, 0.8f, 'C' });
        if (bar == 3) ev.push_back ({ b0 + 3.75 * beat, Cowbell, 0.8f, 'C' });
        if (bar == 5) ev.push_back ({ b0 + 2.5 * beat, Rimshot, 0.9f, 'C' });
        if (bar == 7)
        {   // tom fill on beats 3-4, 16ths
            const int fill[8] = { TomHigh2, TomHigh1, TomMid2, TomMid1, TomLow2, TomLow1, Snare2, Bass };
            for (int i = 0; i < 8; ++i)
                ev.push_back ({ b0 + 2 * beat + i * beat * 0.25, fill[i], 0.75f + 0.03f * i, 'C' });
        }
    }
}

static int demoMode (const std::string& prefix)
{
    // Section layout (seconds). A is 3.0 s/hit (not 1.2 s) so tails can be analysed cleanly.
    const double A0 = 0.0, B0 = 46.0, C0 = 67.0, D0 = 87.0, END = 91.5;
    std::vector<Event> ev;
    for (int i = 0; i < kNumInstruments; ++i)
        ev.push_back ({ A0 + 0.25 + i * 3.0, i, 1.0f, 'A' });
    const float vels[4] = { 0.2f, 0.45f, 0.7f, 1.0f };
    for (int i = 0; i < kNumInstruments; ++i)
        for (int k = 0; k < 4; ++k)
            ev.push_back ({ B0 + 0.1 + i * 1.4 + k * 0.35, i, vels[k], 'B' });
    addGroove (ev, C0);
    for (int k = 0; k < 25; ++k)
        ev.push_back ({ D0 + 0.05 + k * (60.0 / 96.0 / 8.0), Snare1, 0.8f, 'D' });

    const int total = (int) (END * kSR);
    std::unique_ptr<DrumEngine> e (new DrumEngine ((double) kSR));
    setupEngine (*e);
    std::vector<float> L, R;
    renderEvents (*e, ev, total, L, R);

    float pk = 0;
    if (! checkFinite (L, pk) || ! checkFinite (R, pk)) { fprintf (stderr, "FAIL: NaN/Inf in output\n"); return 2; }
    printf ("rendered %.1f s, peak %.3f (%.1f dBFS), channels=%d\n", END, pk, 20 * std::log10 (pk + 1e-12f), kCh);

    // Demo WAVs are 16-bit, peak-normalized to -1 dBFS (same rule for before/after) so they are
    // listenable; raw peak and applied gain are recorded in the .meta.json.
    const float gain = 0.8913f / std::max (pk, 1e-9f);
    { FILE* mf = fopen ((prefix + ".meta.json").c_str(), "w");
      fprintf (mf, "{\"raw_peak\": %.6f, \"raw_peak_dbfs\": %.2f, \"norm_gain\": %.6f, \"channels\": %d, \"sample_rate\": %d}\n",
               pk, 20 * std::log10 (pk + 1e-12f), gain, kCh, kSR); fclose (mf); }
    writeWav ((prefix + ".wav").c_str(), L, R, kSR, kCh, gain);
    const struct { const char* n; double a, b; } secs[4] = {
        { "A", A0, B0 }, { "B", B0, C0 }, { "C", C0, D0 }, { "D", D0, END } };
    for (auto& s : secs)
    {
        std::vector<float> l (L.begin() + (long) (s.a * kSR), L.begin() + (long) (s.b * kSR));
        std::vector<float> r (R.begin() + (long) (s.a * kSR), R.begin() + (long) (s.b * kSR));
        writeWav ((prefix + "_" + s.n + ".wav").c_str(), l, r, kSR, kCh, gain);
    }
    FILE* f = fopen ((prefix + ".events.csv").c_str(), "w");
    fprintf (f, "time_s,instrument,velocity,section,sample_rate\n");
    std::sort (ev.begin(), ev.end(), [] (const Event& a, const Event& b) { return a.t < b.t; });
    for (auto& x : ev) fprintf (f, "%.6f,%d,%.3f,%c,%d\n", x.t, x.inst, x.vel, x.section, kSR);
    fclose (f);
    return 0;
}

static int metricsMode (const std::string& dir)
{
    const float vels[4] = { 0.2f, 0.45f, 0.7f, 1.0f };
    const int total = 3 * kSR;
    for (int i = 0; i < kNumInstruments; ++i)
        for (int k = 0; k < 4; ++k)
        {
            std::unique_ptr<DrumEngine> e (new DrumEngine ((double) kSR));
            setupEngine (*e);
#ifdef OILDRUM_HAS_STEREO
            e->setRoomMix (0.0f);          // metrics are always dry
#endif
            std::vector<float> L, R;
            renderEvents (*e, { { 0.0, i, vels[k], 'M' } }, total, L, R);
            float pk; if (! checkFinite (L, pk) || ! checkFinite (R, pk)) { fprintf (stderr, "FAIL NaN i=%d\n", i); return 2; }
            char name[256]; snprintf (name, sizeof name, "%s/i%02d_v%02d.wav", dir.c_str(), i, (int) std::lround (vels[k] * 100));
            writeWavFloat (name, L, R, kSR, kCh);
        }
    printf ("metrics hits written to %s\n", dir.c_str());
    return 0;
}

// 8 seeds x velocities {0.2, 1.0}: noise-robust velocity->timbre measurement.  files iXX_vYY_sN.wav (0.5 s)
static int veltestMode (const std::string& dir)
{
    const float vels[2] = { 0.2f, 1.0f };
    for (int i = 0; i < kNumInstruments; ++i)
        for (int k = 0; k < 2; ++k)
            for (int s = 0; s < 8; ++s)
            {
                std::unique_ptr<DrumEngine> e (new DrumEngine ((double) kSR));
                setupEngine (*e);
#ifdef OILDRUM_HAS_STEREO
                e->setRoomMix (0.0f); e->seed (0x1234567u + 7919u * (uint32_t) s + 104729u * (uint32_t) i);
#endif
                std::vector<float> L, R;
                renderEvents (*e, { { 0.0, i, vels[k], 'V' } }, kSR / 2, L, R);
                char name[256]; snprintf (name, sizeof name, "%s/i%02d_v%02d_s%d.wav", dir.c_str(), i, (int) std::lround (vels[k] * 100), s);
                writeWavFloat (name, L, R, kSR, kCh);
            }
    printf ("veltest hits written to %s\n", dir.c_str());
    return 0;
}

int main (int argc, char** argv)
{
#if defined(__SSE__)
    _mm_setcsr (_mm_getcsr() | 0x8040);      // flush denormals to zero (plugin uses ScopedNoDenormals)
#endif
    if (argc < 3) { fprintf (stderr, "usage: render_demo demo|metrics OUT\n"); return 1; }
    const std::string mode = argv[1];
    if (argc > 3) gRoom = (float) atof (argv[3]);
    if (mode == "demo")    return demoMode (argv[2]);
    if (mode == "metrics") return metricsMode (argv[2]);
    if (mode == "veltest") return veltestMode (argv[2]);
    return 1;
}
