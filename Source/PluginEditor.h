#pragma once
#include "PluginProcessor.h"

class GainKnobAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GainKnobAudioProcessorEditor(GainKnobAudioProcessor&);
    ~GainKnobAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GainKnobAudioProcessor& processorRef;
    juce::Slider gainSlider;
    juce::Label gainLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainKnobAudioProcessorEditor)
};
