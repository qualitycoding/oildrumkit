#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class OilDrumKitAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit OilDrumKitAudioProcessorEditor (OilDrumKitAudioProcessor&);
    ~OilDrumKitAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    OilDrumKitAudioProcessor& processorRef;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OilDrumKitAudioProcessorEditor)
};
