// KitMap.h
//
// The ONE table that ties each instrument to its MIDI note and its plugin parameter ID.
// The processor (MIDI input, parameter layout), the editor (labels, parameter attachments),
// the tests and the docs all read this table; nothing else may restate a note number.
// Tuning ranges and defaults are NOT here: they come from kitRecipes() in DrumEngine.h.
//
// Notes follow the General MIDI channel-10 key map where it has a matching instrument
// (K-20261008T173540Z): 37 is GM "Side Stick", used here for Rimshot; 40 is GM "Electric Snare".

#pragma once
#include "DrumEngine.h"

namespace oildrum
{

struct InstrumentInfo
{
    int         instrument;   // an Instrument enum value
    const char* paramId;      // plugin parameter ID (stable: saved in host sessions)
    int         midiNote;
};

inline constexpr std::array<InstrumentInfo, kNumInstruments> kKit {{
    { Bass,        "bassPitch",     36 },
    { Snare1,      "snare1Pitch",   38 },
    { Snare2,      "snare2Pitch",   40 },
    { TomLow1,     "tomLow1Pitch",  41 },
    { TomLow2,     "tomLow2Pitch",  43 },
    { TomMid1,     "tomMid1Pitch",  45 },
    { TomMid2,     "tomMid2Pitch",  47 },
    { TomHigh1,    "tomHigh1Pitch", 48 },
    { TomHigh2,    "tomHigh2Pitch", 50 },
    { Splash,      "splashPitch",   55 },
    { Ride,        "ridePitch",     51 },
    { HihatClosed, "hhClosedPitch", 42 },
    { HihatOpen,   "hhOpenPitch",   46 },
    { Cowbell,     "cowbellPitch",  56 },
    { Rimshot,     "rimshotPitch",  37 },
}};

namespace detail
{
    constexpr bool kitIndexedByInstrument()
    {
        for (int i = 0; i < kNumInstruments; ++i)
            if (kKit[(size_t) i].instrument != i) return false;
        return true;
    }

    constexpr bool kitNotesUnique()
    {
        for (int i = 0; i < kNumInstruments; ++i)
            for (int j = i + 1; j < kNumInstruments; ++j)
                if (kKit[(size_t) i].midiNote == kKit[(size_t) j].midiNote) return false;
        return true;
    }
}

static_assert (detail::kitIndexedByInstrument(), "kKit must list instruments in enum order");
static_assert (detail::kitNotesUnique(),         "each MIDI note may trigger only one instrument");

// Returns the instrument for a MIDI note, or -1 if the note is unmapped.
inline int midiNoteToInstrument (int note)
{
    for (const auto& k : kKit)
        if (k.midiNote == note) return k.instrument;
    return -1;
}

} // namespace oildrum
