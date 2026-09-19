// calibrate.cpp -- measures the p90 single-hit peak (v=1, dry, limiter off) per instrument over 24 seeds
// and prints "index peak_p90" lines. Driven by Tools/calibrate.py which updates voicing.json outGain.
#include "../Source/DrumEngine.h"
#include <cstdio>
#include <memory>
#include <vector>
#include <algorithm>
using namespace oildrum;
int main()
{
    const int sr = 48000, n = sr;                 // 1 s (peak occurs in the first few ms .. 100 ms)
    std::vector<float> L ((size_t) n), R ((size_t) n);
    for (int inst = 0; inst < kNumInstruments; ++inst)
    {
        std::vector<float> peaks;
        for (uint32_t s = 1; s <= 24; ++s)
        {
            std::unique_ptr<DrumEngine> e (new DrumEngine ((double) sr));
            e->setHammer (WoodStick); e->setRoomMix (0.0f); e->setLimiter (false); e->seed (s * 2654435761u);
            std::fill (L.begin(), L.end(), 0.0f); std::fill (R.begin(), R.end(), 0.0f);
            e->trigger (inst, 1.0f);
            e->process (L.data(), R.data(), n);
            float pk = 0; for (int i = 0; i < n; ++i) pk = std::max ({ pk, std::fabs (L[(size_t) i]), std::fabs (R[(size_t) i]) });
            peaks.push_back (pk);
        }
        std::sort (peaks.begin(), peaks.end());
        printf ("%d %.6f %.6f\n", inst, peaks[(size_t) (0.9 * (peaks.size() - 1))], peaks.back());
    }
}
