#include "PluginEditor.h"

GainKnobAudioProcessorEditor::GainKnobAudioProcessorEditor(GainKnobAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    gainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 25);
    gainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(gainSlider);

    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    gainLabel.attachToComponent(&gainSlider, false);
    addAndMakeVisible(gainLabel);

    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processorRef.getAPVTS(), "gain", gainSlider);

    setSize(300, 300);
}

void GainKnobAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void GainKnobAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(30);
    bounds.removeFromTop(30); // space for label
    gainSlider.setBounds(bounds);
}
