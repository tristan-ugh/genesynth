#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Style/GeneSynthLookAndFeel.h"
#include "UI/SynthComponents.h"
#include "UI/Views/SynthView.h"
#include "UI/Views/MorphView.h"

class GeneSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    GeneSynthAudioProcessorEditor (GeneSynthAudioProcessor&);
    ~GeneSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void toggleMode();

    GeneSynthAudioProcessor& audioProcessor;
    genesynth::GeneSynthLookAndFeel customLookAndFeel;

    juce::TextButton menuButton{"Menu"};
    juce::TextButton saveButton{"Save"};
    juce::ComboBox presetsBox;
    juce::TextButton modeButton{"Mode"};
    juce::Slider masterVolSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volAttachment;

    std::unique_ptr<genesynth::SynthView> synthView;
    std::unique_ptr<genesynth::MorphView> morphView;

    bool isMorphMode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneSynthAudioProcessorEditor)
};
