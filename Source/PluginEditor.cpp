#include "PluginEditor.h"

using namespace oildrum;

namespace
{
    const juce::Colour bg        (0xff1a1815);
    const juce::Colour panel     (0xff232019);
    const juce::Colour rust      (0xffb5602c);
    const juce::Colour rustLight (0xffe0813f);
    const juce::Colour hazard    (0xffe8b923);
    const juce::Colour textCol   (0xffd8d2c4);

    const char* noteNames[kNumInstruments] = {
        "C1 (36)", "C#2 (37)", "D1 (38)", "E1 (40)",
        "F1 (41)", "G1 (43)", "F#1 (42)", "A#1 (46)",
        "A1 (45)", "B1 (47)", "C2 (48)", "D2 (50)",
        "D#2 (51)", "G2 (55)", "G#2 (56)"
    };
}

OilDrumKitAudioProcessorEditor::OilDrumKitAudioProcessorEditor (OilDrumKitAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    startTimerHz (60);

    title.setText ("OIL DRUM KIT", juce::dontSendNotification);
    title.setFont (juce::FontOptions(26.0f).withStyle("bold"));
    title.setColour (juce::Label::textColourId, rustLight);
    addAndMakeVisible (title);

    subtitle.setText ("tunable steel-shell percussion -- drag each slider to tune that drum", juce::dontSendNotification);
    subtitle.setFont (juce::FontOptions (13.0f));
    subtitle.setColour (juce::Label::textColourId, textCol.withAlpha (0.7f));
    addAndMakeVisible (subtitle);

    static const char* labels[kNumInstruments] = {
        "Bass Drum", "Snare 1", "Snare 2", "Low Tom 1", "Low Tom 2",
        "Mid Tom 1", "Mid Tom 2", "High Tom 1", "High Tom 2",
        "Splash", "Ride", "Hi-Hat Closed", "Hi-Hat Open", "Cowbell", "Rimshot"
    };
    static const char* paramIds[kNumInstruments] = {
        "bassPitch", "snare1Pitch", "snare2Pitch", "tomLow1Pitch", "tomLow2Pitch",
        "tomMid1Pitch", "tomMid2Pitch", "tomHigh1Pitch", "tomHigh2Pitch",
        "splashPitch", "ridePitch", "hhClosedPitch", "hhOpenPitch", "cowbellPitch", "rimshotPitch"
    };

    for (int i = 0; i < kNumInstruments; ++i)
    {
        auto row = std::make_unique<Row>();

        row->nameLabel.setText (labels[i], juce::dontSendNotification);
        row->nameLabel.setFont (juce::FontOptions (15.0f).withStyle ("bold"));
        row->nameLabel.setColour (juce::Label::textColourId, textCol);
        rowsHolder.addAndMakeVisible (row->nameLabel);

        row->noteLabel.setText (juce::String ("MIDI ") + noteNames[i], juce::dontSendNotification);
        row->noteLabel.setFont (juce::FontOptions (11.0f));
        row->noteLabel.setColour (juce::Label::textColourId, textCol.withAlpha (0.5f));
        rowsHolder.addAndMakeVisible (row->noteLabel);

        row->slider.setSliderStyle (juce::Slider::LinearHorizontal);
        row->slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 70, 22);
        row->slider.setColour (juce::Slider::trackColourId, rust);
        row->slider.setColour (juce::Slider::thumbColourId, hazard);
        row->slider.setColour (juce::Slider::textBoxTextColourId, textCol);
        row->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        row->slider.setTextValueSuffix (" Hz");
        rowsHolder.addAndMakeVisible (row->slider);

        row->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processorRef.apvts, paramIds[i], row->slider);

        rows.push_back (std::move (row));
    }

    rowsHolder.setSize (520, kNumInstruments * 56 + 8);
    viewport.setViewedComponent (&rowsHolder, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    setResizable (true, true);
    setSize (560, 560);

    hammerLabel.setText ("HAMMER", juce::dontSendNotification);
    hammerLabel.setFont (juce::FontOptions (12.0f).withStyle ("bold"));
    hammerLabel.setColour (juce::Label::textColourId, rustLight);
    addAndMakeVisible (hammerLabel);

    hammerComboBox.addItem ("Metal Hammer", 1);
    hammerComboBox.addItem ("Wood Stick", 2);
    hammerComboBox.addItem ("Rubber Mallet", 3);
    hammerComboBox.setColour (juce::ComboBox::backgroundColourId, panel);
    hammerComboBox.setColour (juce::ComboBox::textColourId, textCol);
    addAndMakeVisible (hammerComboBox);

    hammerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        processorRef.apvts, "hammerType", hammerComboBox);
}

OilDrumKitAudioProcessorEditor::~OilDrumKitAudioProcessorEditor()
{
    stopTimer();
}

void OilDrumKitAudioProcessorEditor::timerCallback()
{
    for (int i = 0; i < kNumInstruments; ++i)
    {
        // Grab any new hits from the audio thread (and reset the flag to 0)
        float hit = processorRef.hitVelocities[i].exchange (0.0f, std::memory_order_relaxed);
        
        if (hit > 0.0f)
            flashStates[i] = 1.0f; // Max brightness on hit

        // Decay the flash state smoothly
        if (flashStates[i] > 0.0f)
        {
            flashStates[i] -= 0.05f; // Adjust decay speed here
            if (flashStates[i] < 0.0f) 
                flashStates[i] = 0.0f;

            // Blend the thumb color from hazard yellow to solid white
            auto flashColor = juce::Colours::white;
            auto currentColor = hazard.interpolatedWith (flashColor, flashStates[i]);
            
            rows[i]->slider.setColour (juce::Slider::thumbColourId, currentColor);
        }
    }
}

void OilDrumKitAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    // hazard-stripe accent bar under the header
    auto stripe = getLocalBounds().removeFromTop (66).removeFromBottom (4).toFloat();
    const float stripeW = 18.0f;
    juce::Path p;
    for (float x = -stripeW; x < stripe.getWidth() + stripeW; x += stripeW * 2.0f)
    {
        p.addQuadrilateral (x, stripe.getY(), x + stripeW, stripe.getY(),
                             x + stripeW - 10.0f, stripe.getBottom(), x - 10.0f, stripe.getBottom());
    }
    g.setColour (hazard);
    g.fillPath (p);
    g.setColour (juce::Colours::black);
    for (float x = -stripeW * 2; x < stripe.getWidth() + stripeW; x += stripeW * 2.0f)
        g.fillRect (juce::Rectangle<float> (x + stripeW, stripe.getY(), stripeW, stripe.getHeight()));
}

void OilDrumKitAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop (66);
    // Carve out a block on the right side of the header for the hammer selector
    auto hammerArea = header.removeFromRight(150).reduced(0, 10);
    hammerArea.removeFromTop(4); // slight top padding
    hammerLabel.setBounds(hammerArea.removeFromTop(16));
    hammerComboBox.setBounds(hammerArea.removeFromTop(24));
    title.setBounds (header.removeFromTop (36).reduced (14, 4));
    subtitle.setBounds (header.reduced (14, 0));

    viewport.setBounds (area.reduced (8));
    rowsHolder.setSize (viewport.getWidth() - (rowsHolder.getHeight() > viewport.getHeight() ? 10 : 0),
                         kNumInstruments * 56 + 8);

    for (size_t i = 0; i < rows.size(); ++i)
    {
        auto r = juce::Rectangle<int> (12, (int) i * 56 + 4, rowsHolder.getWidth() - 24, 50);
        auto labelRow = r.removeFromTop (20);
        rows[i]->nameLabel.setBounds (labelRow.removeFromLeft (labelRow.getWidth() - 90));
        rows[i]->noteLabel.setBounds (labelRow);
        rows[i]->slider.setBounds (r.removeFromTop (26));
    }
}
