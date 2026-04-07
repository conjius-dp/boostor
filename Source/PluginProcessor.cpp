#include "PluginProcessor.h"
#ifndef GAINKNOB_TESTS
#include "PluginEditor.h"
#endif

GainKnobAudioProcessor::GainKnobAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
GainKnobAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1},
        "Gain",
        juce::NormalisableRange<float>(-100.0f, 24.0f, 0.1f, 3.0f),
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel("dB")));

    return { params.begin(), params.end() };
}

void GainKnobAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    smoothedGain.reset(sampleRate, 0.02);
    float gainDB = apvts.getRawParameterValue("gain")->load();
    smoothedGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(gainDB, -100.0f));
}

void GainKnobAudioProcessor::releaseResources() {}

bool GainKnobAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void GainKnobAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float gainDB = apvts.getRawParameterValue("gain")->load();
    float targetGain = juce::Decibels::decibelsToGain(gainDB, -100.0f);
    smoothedGain.setTargetValue(targetGain);

    if (smoothedGain.isSmoothing())
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            float g = smoothedGain.getNextValue();
            for (int ch = 0; ch < totalNumInputChannels; ++ch)
                buffer.getWritePointer(ch)[sample] *= g;
        }
    }
    else
    {
        float g = smoothedGain.getNextValue();
        for (int ch = 0; ch < totalNumInputChannels; ++ch)
            buffer.applyGain(ch, 0, buffer.getNumSamples(), g);
    }
}

juce::AudioProcessorEditor* GainKnobAudioProcessor::createEditor()
{
#ifndef GAINKNOB_TESTS
    return new GainKnobAudioProcessorEditor(*this);
#else
    return nullptr;
#endif
}

bool GainKnobAudioProcessor::hasEditor() const { return true; }

const juce::String GainKnobAudioProcessor::getName() const { return "GainKnob"; }
bool GainKnobAudioProcessor::acceptsMidi() const { return true; }
bool GainKnobAudioProcessor::producesMidi() const { return false; }
bool GainKnobAudioProcessor::isMidiEffect() const { return false; }
double GainKnobAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int GainKnobAudioProcessor::getNumPrograms() { return 1; }
int GainKnobAudioProcessor::getCurrentProgram() { return 0; }
void GainKnobAudioProcessor::setCurrentProgram(int /*index*/) {}
const juce::String GainKnobAudioProcessor::getProgramName(int /*index*/) { return {}; }
void GainKnobAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void GainKnobAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GainKnobAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GainKnobAudioProcessor();
}
