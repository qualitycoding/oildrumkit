// render_demo.cpp -- a short groove through DrumEngine::render() (the same path the plugin uses).
// No normalisation: the program prints the true peak and exits non-zero if it would clip.
//   g++ -O2 -std=c++17 -I Source tools/render_demo.cpp -o build/render_demo && build/render_demo demo.wav
#include <cstdio>
#include "KitMap.h"
#include "wav.h"
using namespace oildrum;

struct Ev { double beat; int inst; float vel; };

int main (int argc, char** argv)
{
    const int sr = 44100; const double bpm = 100.0, spb = sr * 60.0 / bpm;
    std::vector<Ev> ev;
    for (int bar = 0; bar < 4; ++bar)
    {
        const double b0 = bar * 4;
        for (double b : { 0.0, 2.5 }) ev.push_back ({ b0 + b, Bass, 1.0f });
        if (bar % 2) ev.push_back ({ b0 + 3.5, Bass, 0.8f });
        ev.push_back ({ b0 + 1, bar < 2 ? Snare1 : Snare2, 1.0f }); ev.push_back ({ b0 + 3, bar < 2 ? Snare1 : Snare2, 1.0f });
        for (int e = 0; e < 8; ++e)
        {
            const double b = b0 + e * 0.5;
            if (bar < 2)  ev.push_back ({ b, (e == 7 ? HihatOpen : HihatClosed), e % 2 ? 0.5f : 0.75f });
            else          ev.push_back ({ b, Ride, e % 2 ? 0.45f : 0.7f });
        }
    }
    ev.push_back ({ 0.0, Splash, 0.8f }); ev.push_back ({ 12.0, Splash, 0.8f });
    ev.push_back ({ 6.0, Rimshot, 0.9f }); ev.push_back ({ 14.0, Cowbell, 0.8f }); ev.push_back ({ 14.5, Cowbell, 0.6f });
    const int fill[] = { TomHigh1, TomHigh2, TomMid1, TomMid2, TomLow1, TomLow2 };
    for (int k = 0; k < 6; ++k) ev.push_back ({ 15.0 + k * 0.1666667 , fill[k], 0.9f });
    ev.push_back ({ 16.0, Bass, 1.0f }); ev.push_back ({ 16.0, Splash, 1.0f }); ev.push_back ({ 16.0, Snare1, 1.0f });
    std::sort (ev.begin(), ev.end(), [] (const Ev& a, const Ev& b) { return a.beat < b.beat; });

    DrumEngine eng ((double) sr);
    std::vector<float> out ((size_t) (sr * (17 * 60.0 / bpm + 3.0)));
    size_t pos = 0, next = 0;
    while (pos < out.size())
    {
        size_t until = out.size();
        if (next < ev.size()) until = std::min<size_t> (until, (size_t) (ev[next].beat * spb));
        while (next < ev.size() && (size_t) (ev[next].beat * spb) <= pos) { eng.trigger (ev[next].inst, ev[next].vel); ++next; }
        const size_t n = std::min<size_t> (until > pos ? until - pos : 1, 256);
        eng.render (out.data() + pos, (int) n); pos += n;
    }
    float peak = 0; for (float s : out) peak = std::max (peak, std::fabs (s));
    std::printf ("peak %.4f (no normalisation), %.1f s\n", (double) peak, out.size() / (double) sr);
    if (peak >= 1.0f) return 1;
    return writeWav16 (argc > 1 ? argv[1] : "demo.wav", out, sr) ? 0 : 2;
}
