// render_oneshots.cpp -- one WAV per instrument (default tuning, velocity 1.0, 2.5 s), plus the
// instrument at min and max tuning for the tunable-pitch check. Peaks are printed, not normalised.
//   build/render_oneshots <outdir>
#include <cstdio>
#include <string>
#include "KitMap.h"
#include "wav.h"
using namespace oildrum;

int main (int argc, char** argv)
{
    const std::string dir = argc > 1 ? argv[1] : ".";
    const int sr = 44100; int bad = 0;
    for (int i = 0; i < kNumInstruments; ++i)
        for (int variant = 0; variant < 3; ++variant)
        {
            const Recipe& r = kitRecipes()[(size_t) i];
            const float hz = variant == 0 ? r.defaultHz : variant == 1 ? r.minHz : r.maxHz;
            DrumEngine e ((double) sr); e.setPitchHz (i, hz); e.trigger (i, 1.0f);
            std::vector<float> out ((size_t) (sr * 2.5));
            for (size_t p = 0; p < out.size(); p += 256) e.render (out.data() + p, (int) std::min<size_t> (256, out.size() - p));
            float peak = 0; for (float s : out) peak = std::max (peak, std::fabs (s));
            if (peak >= 1.0f) ++bad;
            std::string name = std::string (instrumentName (i)); for (auto& c : name) if (c == ' ') c = '_';
            const char* tag[] = { "default", "min", "max" };
            char fn[256]; std::snprintf (fn, sizeof fn, "%s/%02d_%s_%s_%.0fHz.wav", dir.c_str(), i, name.c_str(), tag[variant], (double) hz);
            writeWav16 (fn, out, sr);
            std::printf ("%-28s peak %.3f\n", fn, (double) peak);
        }
    return bad ? 1 : 0;
}
