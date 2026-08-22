#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class OilDrumKitAudioProcessorEditor : public juce::AudioProcessorEditor, 
                                       public juce::Timer
{
public:
    explicit OilDrumKitAudioProcessorEditor (OilDrumKitAudioProcessor&);
    ~OilDrumKitAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    OilDrumKitAudioProcessor& processorRef;

    float flashStates[oildrum::kNumInstruments] = {0.0f}; // Tracks animation decay

    struct Row
    {
        juce::Label nameLabel, noteLabel;
        juce::Slider slider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    juce::Viewport viewport;
    juce::Component rowsHolder;
    std::vector<std::unique_ptr<Row>> rows;
    juce::Label title, subtitle;

    juce::Label hammerLabel;
    juce::ComboBox hammerComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> hammerAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OilDrumKitAudioProcessorEditor)
};
