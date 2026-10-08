// export_recipes.cpp -- writes the kit definition (recipes, MIDI map, parameter IDs, constants)
// from the C++ tables to JSON, so the web prototype uses exactly the same numbers as the plugin.
//   g++ -std=c++17 -I Source tools/export_recipes.cpp -o build/export_recipes && build/export_recipes > web/kit.json
#include <cstdio>
#include "KitMap.h"
using namespace oildrum;

static void f (const char* k, float v, bool last = false) { std::printf ("\"%s\":%.9g%s", k, (double) v, last ? "" : ","); }

int main()
{
    std::printf ("{\"masterGain\":%.9g,\"maxPartials\":%d,\"maxInstances\":%d,\"metalRatios\":[", (double) kMasterGain, kMaxPartials, kMaxInstances);
    for (int k = 0; k < kMetalOscs; ++k) std::printf ("%.9g%s", (double) kMetalRatios[(size_t) k], k + 1 < kMetalOscs ? "," : "");
    std::printf ("],\"instruments\":[\n");
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const Recipe& r = kitRecipes()[(size_t) i];
        std::printf ("{\"name\":\"%s\",\"paramId\":\"%s\",\"midiNote\":%d,", instrumentName (i), kKit[(size_t) i].paramId, kKit[(size_t) i].midiNote);
        f ("defaultHz", r.defaultHz); f ("minHz", r.minHz); f ("maxHz", r.maxHz);
        f ("clickAmp", r.clickAmp); f ("clickDecay", r.clickDecay);
        f ("pitchEnvAmt", r.pitchEnvAmt); f ("pitchEnvDecay", r.pitchEnvDecay);
        std::printf ("\"chokeGroup\":%d,", r.chokeGroup);
        std::printf ("\"metal\":{"); f ("amp", r.metal.amp); f ("decay", r.metal.decay); f ("baseScale", r.metal.baseScale);
        f ("hpHz", r.metal.hpHz); f ("noiseMix", r.metal.noiseMix); f ("lpHz", r.metal.lpHz, true);
        std::printf ("},\"partials\":[");
        bool first = true;
        for (const auto& p : r.partials)
        {
            if (p.amp <= 0.0f) continue;
            std::printf ("%s{", first ? "" : ","); first = false;
            f ("ratio", p.ratio); f ("amp", p.amp); f ("decay", p.decay);
            std::printf ("\"noise\":%s,", p.noise ? "true" : "false"); f ("q", p.q, true);
            std::printf ("}");
        }
        std::printf ("]}%s\n", i + 1 < kNumInstruments ? "," : "");
    }
    std::printf ("]}\n");
}
