// plugin_smoke_test.cpp -- drives the REAL OilDrumKitAudioProcessor (JUCE) headlessly.
// Built by the scratch CMake in the development log (JUCE path differs per machine); not part of the plugin build.
//   1. MIDI note-on -> finite, non-silent, non-clipping stereo audio, L != R
//   2. host block sizes larger than announced (resize path), tiny blocks, sample-offset MIDI
//   3. every hammer type
//   4. an OLD-style saved session (without the 3 new parameters) loads and still renders
//   5. new parameters exist with the documented defaults
#include "../Source/PluginProcessor.h"
#include <cstdio>
#include <cmath>

static int failures = 0;
#define CHECK(cond, msg) do { if (! (cond)) { std::printf ("FAIL: %s\n", msg); ++failures; } else std::printf ("ok:   %s\n", msg); } while (0)

static bool render (OilDrumKitAudioProcessor& p, int blockSize, int blocks, int note, int offset, float& peak, float& diffLR, bool& finite)
{
    peak = 0.0f; diffLR = 0.0f; finite = true;
    juce::AudioBuffer<float> buf (2, blockSize);
    for (int b = 0; b < blocks; ++b)
    {
        buf.clear();
        juce::MidiBuffer midi;
        if (b == 0) midi.addEvent (juce::MidiMessage::noteOn (10, note, (juce::uint8) 110), offset);
        p.processBlock (buf, midi);
        for (int i = 0; i < blockSize; ++i)
        {
            const float l = buf.getSample (0, i), r = buf.getSample (1, i);
            if (! std::isfinite (l) || ! std::isfinite (r)) finite = false;
            peak = std::max ({ peak, std::fabs (l), std::fabs (r) }); diffLR += std::fabs (l - r);
        }
    }
    return true;
}

int main()
{
    juce::ScopedJuceInitialiser_GUI init;
    OilDrumKitAudioProcessor p;
    p.setPlayConfigDetails (0, 2, 48000.0, 512);
    p.prepareToPlay (48000.0, 512);

    float pk, d; bool fin;
    render (p, 512, 200, 36, 100, pk, d, fin);
    CHECK (fin, "kick: output finite"); CHECK (pk > 0.05f, "kick: audible"); CHECK (pk < 0.97f, "kick: below soft ceiling");
    std::printf ("      kick peak %.3f, sum|L-R| %.2f\n", pk, d); CHECK (d > 0.1f, "kick: stereo channels differ");

    for (int note : { 38, 42, 46, 51, 55, 56, 37, 41, 48 })
    {   render (p, 512, 60, note, 3, pk, d, fin); char m[64]; std::snprintf (m, sizeof m, "note %d finite & audible", note); CHECK (fin && pk > 0.02f, m); }

    render (p, 4096, 20, 38, 4000, pk, d, fin);   CHECK (fin && pk > 0.02f, "block 4096 > announced 512 (resize path)");
    render (p, 32, 400, 38, 31, pk, d, fin);      CHECK (fin && pk > 0.02f, "block 32, MIDI at last sample");

    for (int h = 0; h < 3; ++h)
    {
        auto* prm = p.apvts.getParameter ("hammerType"); prm->setValueNotifyingHost (prm->convertTo0to1 ((float) h));
        render (p, 512, 30, 38, 0, pk, d, fin); char m[64]; std::snprintf (m, sizeof m, "hammer %d finite & audible", h); CHECK (fin && pk > 0.01f, m);
    }

    CHECK (p.apvts.getParameter ("strikePos") != nullptr && p.apvts.getParameter ("damping") != nullptr && p.apvts.getParameter ("roomMix") != nullptr, "new params exist");
    CHECK (std::fabs (p.apvts.getRawParameterValue ("roomMix")->load() - 0.12f) < 1e-4f, "roomMix default 0.12");

    // ---- old-session compatibility: strip the 3 new params from a saved state and load it
    juce::MemoryBlock mb; p.getStateInformation (mb);
    if (auto xml = juce::AudioProcessor::getXmlFromBinary (mb.getData(), (int) mb.getSize()))
    {
        for (int i = xml->getNumChildElements() - 1; i >= 0; --i)
        {
            auto id = xml->getChildElement (i)->getStringAttribute ("id");
            if (id == "strikePos" || id == "damping" || id == "roomMix") xml->removeChildElement (xml->getChildElement (i), true);
        }
        juce::MemoryBlock old; juce::AudioProcessor::copyXmlToBinary (*xml, old);
        OilDrumKitAudioProcessor q; q.setPlayConfigDetails (0, 2, 44100.0, 256); q.prepareToPlay (44100.0, 256);
        q.setStateInformation (old.getData(), (int) old.getSize());
        render (q, 256, 100, 36, 0, pk, d, fin);
        CHECK (fin && pk > 0.05f, "old-style session (no new params) loads and renders at 44.1 kHz");
        CHECK (q.apvts.getParameter ("roomMix") != nullptr, "new params still present after loading old state");
    }
    else { std::printf ("FAIL: could not read state xml\n"); ++failures; }

    std::printf ("\n%s (%d failure%s)\n", failures ? "SMOKE TEST FAILED" : "SMOKE TEST PASSED", failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
