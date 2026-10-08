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
}

OilDrumKitAudioProcessorEditor::OilDrumKitAudioProcessorEditor (OilDrumKitAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    title.setText ("OIL DRUM KIT", juce::dontSendNotification);
    title.setFont (juce::Font (26.0f, juce::Font::bold));
    title.setColour (juce::Label::textColourId, rustLight);
    addAndMakeVisible (title);

    subtitle.setText ("tunable steel-shell percussion -- drag each slider to tune that drum", juce::dontSendNotification);
    subtitle.setFont (juce::Font (13.0f));
    subtitle.setColour (juce::Label::textColourId, textCol.withAlpha (0.7f));
    addAndMakeVisible (subtitle);

    for (int i = 0; i < kNumInstruments; ++i)
    {
        auto row = std::make_unique<Row>();

        row->nameLabel.setText (instrumentName (i), juce::dontSendNotification);
        row->nameLabel.setFont (juce::Font (15.0f, juce::Font::bold));
        row->nameLabel.setColour (juce::Label::textColourId, textCol);
        rowsHolder.addAndMakeVisible (row->nameLabel);

        row->noteLabel.setText (juce::String ("MIDI note ") + juce::String (kKit[(size_t) i].midiNote), juce::dontSendNotification);
        row->noteLabel.setFont (juce::Font (11.0f));
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
            processorRef.apvts, kKit[(size_t) i].paramId, row->slider);

        rows.push_back (std::move (row));
    }

    rowsHolder.setSize (520, kNumInstruments * 56 + 8);
    viewport.setViewedComponent (&rowsHolder, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    setResizable (true, true);
    setSize (560, 560);
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
