#pragma once
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
// Minimal 16-bit mono PCM WAV writer. No normalisation: the caller passes the engine's render() output.
inline bool writeWav16 (const std::string& path, const std::vector<float>& x, int sampleRate)
{
    FILE* f = std::fopen (path.c_str(), "wb");
    if (! f) return false;
    const uint32_t dataBytes = (uint32_t) (x.size() * 2), rate = (uint32_t) sampleRate;
    auto u32 = [&] (uint32_t v) { std::fwrite (&v, 4, 1, f); };
    auto u16 = [&] (uint16_t v) { std::fwrite (&v, 2, 1, f); };
    std::fwrite ("RIFF", 1, 4, f); u32 (36 + dataBytes); std::fwrite ("WAVEfmt ", 1, 8, f);
    u32 (16); u16 (1); u16 (1); u32 (rate); u32 (rate * 2); u16 (2); u16 (16);
    std::fwrite ("data", 1, 4, f); u32 (dataBytes);
    for (float s : x) { const int16_t q = (int16_t) std::lrintf (std::clamp (s, -1.0f, 1.0f) * 32767.0f); std::fwrite (&q, 2, 1, f); }
    std::fclose (f);
    return true;
}
