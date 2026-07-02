#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Style/GeneSynthLookAndFeel.h"
#include "UI/SynthComponents.h"

class GeneSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GeneSynthAudioProcessorEditor (GeneSynthAudioProcessor&);
    ~GeneSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GeneSynthAudioProcessor& audioProcessor;
    genesynth::GeneSynthLookAndFeel customLookAndFeel;

    juce::TextButton menuButton{"Menu"};
    juce::TextButton saveButton{"Save"};
    juce::ComboBox presetsBox;
    juce::TextButton modeButton{"Mode"};
    juce::Slider masterVolSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volAttachment;

    std::unique_ptr<genesynth::OscASection> oscA;
    std::unique_ptr<genesynth::OscBSection> oscB;
    std::unique_ptr<genesynth::NoiseSection> noise;
    std::unique_ptr<genesynth::FilterSectionUI> filter;
    
    std::unique_ptr<genesynth::ModulationSectionUI> modulation;
    std::unique_ptr<genesynth::GlobalEnvSection> globalEnv;
    std::unique_ptr<genesynth::SoftClipperSection> softClipper;
    std::unique_ptr<genesynth::ReverbSection> reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneSynthAudioProcessorEditor)
};
