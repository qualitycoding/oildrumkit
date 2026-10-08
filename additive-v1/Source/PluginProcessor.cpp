#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace oildrum;

OilDrumKitAudioProcessor::OilDrumKitAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    for (int i = 0; i < kNumInstruments; ++i)
        pitchParams[i] = apvts.getRawParameterValue (kKit[(size_t) i].paramId);
}

juce::AudioProcessorValueTreeState::ParameterLayout OilDrumKitAudioProcessor::createLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const auto& r = kitRecipes()[(size_t) i];
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { kKit[(size_t) i].paramId, 1 }, instrumentName (i),
            juce::NormalisableRange<float> (r.minHz, r.maxHz, 0.1f, 0.5f), // skewed for musical tuning feel
            r.defaultHz,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));
    }
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

    // Pitch is latched when a drum is struck: moving a knob changes the NEXT hit,
    // not a note that is already ringing (tested in tests/test_engine.cpp, section 5).
    for (int i = 0; i < kNumInstruments; ++i)
        engine.setPitchHz (i, pitchParams[i]->load());

    // Note: events are applied at the start of the block (not sample-accurate).
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            const int inst = oildrum::midiNoteToInstrument (msg.getNoteNumber());
            if (inst >= 0)
                engine.trigger (inst, msg.getFloatVelocity());
        }
    }

    const int numSamples = buffer.getNumSamples();
    if ((int) scratch.size() < numSamples)
        scratch.resize ((size_t) numSamples);
    engine.render (scratch.data(), numSamples);   // includes the master gain / soft clip

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
