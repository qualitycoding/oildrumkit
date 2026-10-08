#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace oildrum;

// Parameter IDs and display names, in Instrument enum order. Ranges and defaults are NOT stated here:
// they are read from kVoicing (Source/Voicing.h, generated from Tools/voicing.json), the same table the
// engine clamps against, so the two cannot disagree.
struct ParamSpec { const char* id; const char* label; };
static const ParamSpec kParamSpecs[kNumInstruments] = {
    { "bassPitch",    "Bass Drum" },
    { "snare1Pitch",  "Snare 1" },
    { "snare2Pitch",  "Snare 2" },
    { "tomLow1Pitch", "Low Tom 1" },
    { "tomLow2Pitch", "Low Tom 2" },
    { "tomMid1Pitch", "Mid Tom 1" },
    { "tomMid2Pitch", "Mid Tom 2" },
    { "tomHigh1Pitch","High Tom 1" },
    { "tomHigh2Pitch","High Tom 2" },
    { "splashPitch",  "Splash" },
    { "ridePitch",    "Ride" },
    { "hhClosedPitch","Hi-Hat Closed" },
    { "hhOpenPitch",  "Hi-Hat Open" },
    { "cowbellPitch", "Cowbell" },
    { "rimshotPitch", "Rimshot" },
};

OilDrumKitAudioProcessor::OilDrumKitAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < kNumInstruments; ++i)
        pitchParams[i] = apvts.getRawParameterValue (kParamSpecs[i].id);
    hammerParam = apvts.getRawParameterValue ("hammerType");
    strikePosParam = apvts.getRawParameterValue ("strikePos");
    dampingParam   = apvts.getRawParameterValue ("damping");
    roomMixParam   = apvts.getRawParameterValue ("roomMix");
}

juce::AudioProcessorValueTreeState::ParameterLayout OilDrumKitAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const auto& spec = kParamSpecs[i];
        const auto& rec  = kVoicing[i];
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { spec.id, 1 }, spec.label,
            juce::NormalisableRange<float> (rec.minHz, rec.maxHz, 0.1f, 0.5f), // skewed for musical tuning feel
            rec.defaultHz,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    }
    
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "hammerType", 1 }, "Hammer Type",
        juce::StringArray { "Metal Hammer", "Wood Stick", "Rubber Mallet" }, 1)); // 1 = default Wood Stick

    // Parameters added by the modal-engine rewrite. APPENDED after the original ones (never reorder:
    // saved sessions from the old version still load, and these fall back to their defaults).
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "strikePos", 1 }, "Strike Position",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.5f));            // 0 = centre-ward, 1 = rim-ward
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "damping", 1 }, "Damping",
        juce::NormalisableRange<float> (0.5f, 2.0f, 0.01f, 0.5f), 1.0f));       // multiplies every decay rate
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "roomMix", 1 }, "Room",
        juce::NormalisableRange<float> (0.0f, 0.4f, 0.001f), 0.12f));

    return { params.begin(), params.end() };
}

void OilDrumKitAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.setSampleRate (sampleRate);
    scratchL.assign ((size_t) samplesPerBlock, 0.0f);
    scratchR.assign ((size_t) samplesPerBlock, 0.0f);
}

void OilDrumKitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    engine.setHammer(static_cast<oildrum::HammerType>((int)hammerParam->load()));
    engine.setStrikePos (strikePosParam->load());
    engine.setDamping   (dampingParam->load());
    engine.setRoomMix   (roomMixParam->load());

    for (int i = 0; i < kNumInstruments; ++i)
        engine.setPitchHz (i, pitchParams[i]->load());

    const int numSamples = buffer.getNumSamples();
    if ((int) scratchL.size() < numSamples)
    {
        // Only happens if a host exceeds the block size announced in prepareToPlay.
        scratchL.resize ((size_t) numSamples);
        scratchR.resize ((size_t) numSamples);
    }

    // The engine ADDS into its output buffers, so clear them first
    std::fill (scratchL.begin(), scratchL.begin() + numSamples, 0.0f);
    std::fill (scratchR.begin(), scratchR.begin() + numSamples, 0.0f);

    int currentSample = 0;

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int eventSample = metadata.samplePosition;

        // 1. Render audio up to the exact sample of this MIDI event
        if (eventSample > currentSample)
        {
            const int samplesToRender = eventSample - currentSample;
            engine.process (scratchL.data() + currentSample, scratchR.data() + currentSample, samplesToRender);
            currentSample = eventSample;
        }

        // 2. Trigger the instrument
        if (msg.isNoteOn())
        {
            const int inst = midiNoteToInstrument (msg.getNoteNumber());
            if (inst >= 0) {
                engine.trigger (inst, msg.getFloatVelocity());
                hitVelocities[inst].store (msg.getFloatVelocity(), std::memory_order_relaxed);
            }
        }
    }

    // 3. Render any remaining samples in the block after the last MIDI event
    if (currentSample < numSamples)
    {
        engine.process (scratchL.data() + currentSample, scratchR.data() + currentSample, numSamples - currentSample);
    }

    // 4. Copy the rendered stereo pair to the output channels (extra channels, if any, get the mono sum)
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        if (ch == 0)      buffer.copyFrom (ch, 0, scratchL.data(), numSamples);
        else if (ch == 1) buffer.copyFrom (ch, 0, scratchR.data(), numSamples);
        else
        {
            buffer.clear (ch, 0, numSamples);
            buffer.addFrom (ch, 0, scratchL.data(), numSamples, 0.5f);
            buffer.addFrom (ch, 0, scratchR.data(), numSamples, 0.5f);
        }
    }
}

juce::AudioProcessorEditor* OilDrumKitAudioProcessor::createEditor()
{
    return new OilDrumKitAudioProcessorEditor (*this);
}

void OilDrumKitAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void OilDrumKitAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates instances of the plugin for each host.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OilDrumKitAudioProcessor();
}
