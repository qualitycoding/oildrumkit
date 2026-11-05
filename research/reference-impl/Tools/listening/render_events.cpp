// render_events.cpp -- deterministic offline renderer driven by a CSV event file (listening phase, plan D-004).
//
//   render_events --clips    EVENTS.csv OUT_DIR   one engine per row -> OUT_DIR/<stem>.wav  (stereo float32, 48 kHz, dry unless room>0)
//   render_events --timeline EVENTS.csv OUT.wav   one engine for all rows, ordered by time_s -> OUT.wav (16-bit, peak-normalised to -1 dBFS)
//
// CSV header (exactly): stem,time_s,instrument,velocity,hammer,strike,damping,room,pitch_hz,seed,duration_s
//   hammer metal|wood|rubber; strike 0..1 (0.5 neutral); damping 0.25..4; room 0..0.5; pitch_hz empty = instrument default;
//   seed empty = engine default; duration_s empty = 3.0 (clips mode only).  A pitch outside the instrument's range is clamped by
//   the engine; this tool prints "pitch_clamped <stem> <asked> <used>" on stderr when that happens.
// OUT_DIR is created if missing.
// A row with defaults (wood, 0.5, 1, 0, empty, empty, 3.0) reproduces `render_demo metrics` bit for bit (test T-L06).
#include "../../Source/DrumEngine.h"
#include "../wav.h"
#if defined(__SSE__)
#include <xmmintrin.h>
#endif
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace oildrum;
static const int kSR = 48000, kBlock = 512;

struct Row { std::string stem; double t; int inst; float vel; HammerType hammer; float strike, damping, room; bool hasPitch; float pitch; bool hasSeed; uint32_t seed; double dur; };

static std::vector<std::string> split (const std::string& s)
{
    std::vector<std::string> out; std::string cur; std::stringstream ss (s);
    while (std::getline (ss, cur, ',')) out.push_back (cur);
    if (! s.empty() && s.back() == ',') out.push_back ("");
    return out;
}

static bool readRows (const char* path, std::vector<Row>& rows)
{
    std::ifstream f (path);
    std::string line;
    if (! std::getline (f, line)) { fprintf (stderr, "empty file\n"); return false; }
    while (! line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
    if (line != "stem,time_s,instrument,velocity,hammer,strike,damping,room,pitch_hz,seed,duration_s") { fprintf (stderr, "bad header: %s\n", line.c_str()); return false; }
    int n = 1;
    while (std::getline (f, line))
    {
        ++n;
        while (! line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
        if (line.empty()) continue;
        auto c = split (line);
        if (c.size() != 11) { fprintf (stderr, "line %d: %zu fields, expected 11\n", n, c.size()); return false; }
        Row r;
        r.stem = c[0]; r.t = atof (c[1].c_str()); r.inst = atoi (c[2].c_str()); r.vel = (float) atof (c[3].c_str());
        if (c[4] == "metal") r.hammer = MetalHammer; else if (c[4] == "wood") r.hammer = WoodStick; else if (c[4] == "rubber") r.hammer = RubberMallet;
        else { fprintf (stderr, "line %d: unknown hammer '%s'\n", n, c[4].c_str()); return false; }
        r.strike = (float) atof (c[5].c_str()); r.damping = (float) atof (c[6].c_str()); r.room = (float) atof (c[7].c_str());
        r.hasPitch = ! c[8].empty(); r.pitch = r.hasPitch ? (float) atof (c[8].c_str()) : 0.0f;
        r.hasSeed = ! c[9].empty(); r.seed = r.hasSeed ? (uint32_t) strtoul (c[9].c_str(), nullptr, 10) : 0u;
        r.dur = c[10].empty() ? 3.0 : atof (c[10].c_str());
        if (r.inst < 0 || r.inst >= kNumInstruments) { fprintf (stderr, "line %d: instrument %d out of range\n", n, r.inst); return false; }
        rows.push_back (r);
    }
    return ! rows.empty();
}

static void applyRow (DrumEngine& e, const Row& r)
{
    e.setHammer (r.hammer); e.setStrikePos (r.strike); e.setDamping (r.damping); e.setRoomMix (r.room);
    if (r.hasSeed) e.seed (r.seed);
    if (r.hasPitch)
    {
        const Recipe& rc = e.getRecipe (r.inst);
        if (r.pitch < rc.minHz || r.pitch > rc.maxHz)
            fprintf (stderr, "pitch_clamped %s %.3f %.3f\n", r.stem.c_str(), r.pitch, std::clamp (r.pitch, rc.minHz, rc.maxHz));
        e.setPitchHz (r.inst, r.pitch);
    }
}

// Same block structure as render_demo's renderEvents so single-hit output is identical.
static void renderTimed (DrumEngine& e, const std::vector<Row>& rows, int total, std::vector<float>& L, std::vector<float>& R)
{
    L.assign ((size_t) total, 0.0f); R.assign ((size_t) total, 0.0f);
    size_t next = 0; int pos = 0;
    while (pos < total)
    {
        int blockEnd = std::min (pos + kBlock, total), cur = pos;
        while (true)
        {
            int evSample = blockEnd;
            if (next < rows.size()) evSample = std::min (blockEnd, (int) std::llround (rows[next].t * kSR));
            if (evSample > cur) { e.process (L.data() + cur, R.data() + cur, evSample - cur); cur = evSample; }
            if (next < rows.size() && (int) std::llround (rows[next].t * kSR) <= cur)
            {
                applyRow (e, rows[next]); e.trigger (rows[next].inst, rows[next].vel); ++next; continue;
            }
            if (cur >= blockEnd) break;
        }
        pos = blockEnd;
    }
}

int main (int argc, char** argv)
{
#if defined(__SSE__)
    _mm_setcsr (_mm_getcsr() | 0x8040);      // flush denormals to zero, as render_demo and the plugin do
#endif
    if (argc != 4) { fprintf (stderr, "usage: render_events --clips|--timeline EVENTS.csv OUT\n"); return 1; }
    const std::string mode = argv[1];
    std::vector<Row> rows;
    if (! readRows (argv[2], rows)) return 1;
    if (mode == "--clips")
    {
        std::filesystem::create_directories (argv[3]);
        for (const Row& r : rows)
        {
            std::unique_ptr<DrumEngine> e (new DrumEngine ((double) kSR));
            std::vector<float> L, R;
            Row one = r;                               // event time is honoured inside the clip
            renderTimed (*e, { one }, (int) std::llround (one.dur * kSR), L, R);
            const std::string name = std::string (argv[3]) + "/" + r.stem + ".wav";
            if (! writeWavFloat (name.c_str(), L, R, kSR, 2)) { fprintf (stderr, "cannot write %s\n", name.c_str()); return 1; }
        }
        return 0;
    }
    if (mode == "--timeline")
    {
        std::sort (rows.begin(), rows.end(), [] (const Row& a, const Row& b) { return a.t < b.t; });
        std::unique_ptr<DrumEngine> e (new DrumEngine ((double) kSR));
        double end = 0.0; for (const Row& r : rows) end = std::max (end, r.t + r.dur);
        std::vector<float> L, R;
        renderTimed (*e, rows, (int) std::llround (end * kSR), L, R);
        float pk = 1e-9f; for (size_t i = 0; i < L.size(); ++i) pk = std::max ({ pk, std::fabs (L[i]), std::fabs (R[i]) });
        if (! writeWav (argv[3], L, R, kSR, 2, 0.8913f / pk)) { fprintf (stderr, "cannot write %s\n", argv[3]); return 1; }
        printf ("timeline %.2f s, raw peak %.4f\n", end, pk);
        return 0;
    }
    fprintf (stderr, "unknown mode %s\n", mode.c_str());
    return 1;
}
