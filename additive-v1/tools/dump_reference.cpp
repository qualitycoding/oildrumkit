// dump_reference.cpp -- renders fixed scenarios through DrumEngine::render() and writes raw float32
// files, used by tests/test_web_parity.js to check the JavaScript port sample-by-sample.
//   usage: dump_reference <outdir> [sampleRate]
#include <cstdio>
#include <string>
#include <vector>
#include "KitMap.h"
using namespace oildrum;

static void dump (const std::string& path, std::vector<float>& v)
{
    FILE* fp = std::fopen (path.c_str(), "wb");
    std::fwrite (v.data(), sizeof (float), v.size(), fp);
    std::fclose (fp);
}

int main (int argc, char** argv)
{
    const std::string dir = argc > 1 ? argv[1] : ".";
    const double sr = argc > 2 ? std::atof (argv[2]) : 44100.0;
    const int n = (int) (sr * 1.5);
    // 1) every instrument alone, velocity 1.0 at default tuning, block size 256 (as a host might)
    for (int i = 0; i < kNumInstruments; ++i)
    {
        DrumEngine e (sr);
        std::vector<float> out ((size_t) n);
        e.trigger (i, 1.0f);
        for (int pos = 0; pos < n; pos += 256) e.render (out.data() + pos, std::min (256, n - pos));
        dump (dir + "/inst" + std::to_string (i) + ".f32", out);
    }
    // 2) a groove: retuned snare, open hat then closed hat (choke), a flam, bass+ride+tom at once
    {
        DrumEngine e (sr);
        std::vector<float> out ((size_t) (sr * 2));
        e.setPitchHz (Snare1, 250.0f);
        e.setPitchHz (TomLow1, 70.0f);
        for (int pos = 0, blk = 0; pos < (int) out.size(); pos += 256, ++blk)
        {
            const int t = pos;
            if (t == 0)        { e.trigger (Bass, 1.0f); e.trigger (Ride, 0.8f); e.trigger (TomLow1, 0.9f); e.trigger (Snare1, 1.0f); }
            if (t == 256 * 40) e.trigger (HihatOpen, 0.7f);
            if (t == 256 * 60) e.trigger (HihatClosed, 0.7f);
            if (t == 256 * 80) { e.trigger (Snare2, 0.6f); e.trigger (Snare2, 1.0f); }
            e.render (out.data() + pos, std::min (256, (int) out.size() - pos));
        }
        dump (dir + "/groove.f32", out);
    }
}
