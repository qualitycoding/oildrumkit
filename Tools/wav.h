// wav.h -- minimal 16-bit PCM WAV writer (mono or stereo) used by the offline tools.
#pragma once
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>

// 16-bit PCM, samples multiplied by `gain` then hard-clipped to [-1,1].
inline bool writeWav (const char* path, const std::vector<float>& L, const std::vector<float>& R,
                      int sr, int channels, float gain = 1.0f)
{
    const int n = (int) L.size();
    const int bits = 16;
    const int dataBytes = n * channels * bits / 8;
    FILE* f = fopen (path, "wb");
    if (! f) return false;
    auto u32 = [&] (uint32_t v) { fwrite (&v, 4, 1, f); };
    auto u16 = [&] (uint16_t v) { fwrite (&v, 2, 1, f); };
    fwrite ("RIFF", 1, 4, f); u32 (36 + dataBytes); fwrite ("WAVE", 1, 4, f);
    fwrite ("fmt ", 1, 4, f); u32 (16); u16 (1); u16 ((uint16_t) channels); u32 ((uint32_t) sr);
    u32 ((uint32_t) (sr * channels * bits / 8)); u16 ((uint16_t) (channels * bits / 8)); u16 (bits);
    fwrite ("data", 1, 4, f); u32 ((uint32_t) dataBytes);
    for (int i = 0; i < n; ++i)
        for (int c = 0; c < channels; ++c)
        {
            float v = gain * (c == 0 ? L[(size_t) i] : R[(size_t) i]);
            v = v > 1.f ? 1.f : (v < -1.f ? -1.f : v);
            int16_t s = (int16_t) lrintf (v * 32767.f);
            fwrite (&s, 2, 1, f);
        }
    fclose (f);
    return true;
}

// 32-bit IEEE float WAV (format tag 3): exact, unclipped -- used for analysis renders.
inline bool writeWavFloat (const char* path, const std::vector<float>& L, const std::vector<float>& R,
                           int sr, int channels)
{
    const int n = (int) L.size();
    const int dataBytes = n * channels * 4;
    FILE* f = fopen (path, "wb");
    if (! f) return false;
    auto u32 = [&] (uint32_t v) { fwrite (&v, 4, 1, f); };
    auto u16 = [&] (uint16_t v) { fwrite (&v, 2, 1, f); };
    fwrite ("RIFF", 1, 4, f); u32 (36 + dataBytes); fwrite ("WAVE", 1, 4, f);
    fwrite ("fmt ", 1, 4, f); u32 (16); u16 (3); u16 ((uint16_t) channels); u32 ((uint32_t) sr);
    u32 ((uint32_t) (sr * channels * 4)); u16 ((uint16_t) (channels * 4)); u16 (32);
    fwrite ("data", 1, 4, f); u32 ((uint32_t) dataBytes);
    for (int i = 0; i < n; ++i)
        for (int c = 0; c < channels; ++c)
        { float v = (c == 0 ? L[(size_t) i] : R[(size_t) i]); fwrite (&v, 4, 1, f); }
    fclose (f);
    return true;
}
