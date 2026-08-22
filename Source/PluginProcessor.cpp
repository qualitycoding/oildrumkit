#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace oildrum;

// Parameter IDs, in Instrument enum order. Ranges/defaults mirror the
// defaults baked into DrumEngine::buildDefaultRecipes() -- if you retune a
// recipe there, update the matching entry here too.
struct ParamSpec { const char* id; const char* label; float minHz; float maxHz; float defaultHz; };
static const ParamSpec kParamSpecs[kNumInstruments] = {
    { "bassPitch",    "Bass Drum",      30, 120,  55 },
    { "snare1Pitch",  "Snare 1",       100, 400, 190 },
    { "snare2Pitch",  "Snare 2",        90, 350, 160 },
    { "tomLow1Pitch", "Low Tom 1",      47, 209,  95 },
    { "tomLow2Pitch", "Low Tom 2",      52, 231, 105 },
    { "tomMid1Pitch", "Mid Tom 1",      72, 319, 145 },
    { "tomMid2Pitch", "Mid Tom 2",      80, 352, 160 },
    { "tomHigh1Pitch","High Tom 1",    105, 462, 210 },
    { "tomHigh2Pitch","High Tom 2",    115, 506, 230 },
    { "splashPitch",  "Splash",        250,1000, 500 },
    { "ridePitch",    "Ride",          200, 800, 420 },
    { "hhClosedPitch","Hi-Hat Closed", 250, 900, 500 },
    { "hhOpenPitch",  "Hi-Hat Open",   250, 900, 500 },
    { "cowbellPitch", "Cowbell",       300,1100, 560 },
    { "rimshotPitch", "Rimshot",       180, 700, 350 },
};

OilDrumKitAudioProcessor::OilDrumKitAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < kNumInstruments; ++i)
        pitchParams[i] = apvts.getRawParameterValue (kParamSpecs[i].id);
    hammerParam = apvts.getRawParameterValue ("hammerType");
}

juce::AudioProcessorValueTreeState::ParameterLayout OilDrumKitAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    for (auto& spec : kParamSpecs)
    {
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { spec.id, 1 }, spec.label,
            juce::NormalisableRange<float> (spec.minHz, spec.maxHz, 0.1f, 0.5f), // skewed for musical tuning feel
            spec.defaultHz,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    }
    
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "hammerType", 1 }, "Hammer Type",
        juce::StringArray { "Metal Hammer", "Wood Stick", "Rubber Mallet" }, 1)); // 1 = default Wood Stick

    return { params.begin(), params.end() };
}

void OilDrumKitAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.setSampleRate (sampleRate);
    scratch.assign ((size_t) samplesPerBlock, 0.0f);
}

void OilDrumKitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    engine.setHammer(static_cast<oildrum::HammerType>((int)hammerParam->load()));

    for (int i = 0; i < kNumInstruments; ++i)
        engine.setPitchHz (i, pitchParams[i]->load());

    const int numSamples = buffer.getNumSamples();
    if ((int) scratch.size() < numSamples)
        scratch.resize ((size_t) numSamples);
    
    // Clear the scratch buffer before adding to it
    std::fill (scratch.begin(), scratch.begin() + numSamples, 0.0f);

    int currentSample = 0;

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int eventSample = metadata.samplePosition;

        // 1. Render audio up to the exact sample of this MIDI event
        if (eventSample > currentSample)
        {
            const int samplesToRender = eventSample - currentSample;
            engine.process (scratch.data() + currentSample, samplesToRender);
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
        engine.process (scratch.data() + currentSample, numSamples - currentSample);
    }

    // 4. Copy the fully rendered scratch buffer to the output channels
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        buffer.copyFrom (ch, 0, scratch.data(), numSamples);
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
