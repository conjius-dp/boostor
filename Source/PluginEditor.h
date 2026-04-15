#pragma once
#include "PluginProcessor.h"
#include "KnobDesign.h"
#include "BinaryData.h"

class BoostorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit BoostorAudioProcessorEditor(BoostorAudioProcessor&);
    ~BoostorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    BoostorAudioProcessor& processorRef;
    ConjusKnobLookAndFeel conjusLAF;
    juce::Slider gainSlider;
    juce::Label gainLabel;
    juce::Label latencyLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
    juce::Image logoImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BoostorAudioProcessorEditor)
};
