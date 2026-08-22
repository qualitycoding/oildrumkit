#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DrumEngine.h"

// MIDI note map (loosely follows the General MIDI drum-channel conventions
// so it drops onto an existing groove/pattern with minimal remapping).
//   36 Bass Drum        41 Low Tom 1        51 Ride
//   37 Rimshot          43 Low Tom 2        55 Splash
//   38 Snare 1          45 Mid Tom 1        56 Cowbell
//   40 Snare 2          47 Mid Tom 2
//   42 Hi-Hat Closed    48 High Tom 1
//   46 Hi-Hat Open      50 High Tom 2
inline int midiNoteToInstrument (int note)
{
    using namespace oildrum;
    switch (note)
    {
        case 36: return Bass;
        case 37: return Rimshot;
        case 38: return Snare1;
        case 40: return Snare2;
        case 41: return TomLow1;
        case 43: return TomLow2;
        case 42: return HihatClosed;
        case 46: return HihatOpen;
        case 45: return TomMid1;
        case 47: return TomMid2;
        case 48: return TomHigh1;
        case 50: return TomHigh2;
        case 51: return Ride;
        case 55: return Splash;
        case 56: return Cowbell;
        default: return -1;
    }
}

class OilDrumKitAudioProcessor : public juce::AudioProcessor
{
public:
    OilDrumKitAudioProcessor();
    ~OilDrumKitAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    std::atomic<float> hitVelocities[oildrum::kNumInstruments] {};
    
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Oil Drum Kit"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    oildrum::DrumEngine engine { 44100.0 };
    std::array<std::atomic<float>*, oildrum::kNumInstruments> pitchParams {};
    std::atomic<float>* hammerParam { nullptr };
    std::vector<float> scratch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OilDrumKitAudioProcessor)
};
