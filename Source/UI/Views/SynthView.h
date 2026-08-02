#pragma once

#include <JuceHeader.h>
#include "../SynthComponents.h"

namespace genesynth {

class SynthView : public juce::Component
{
public:
    SynthView(juce::AudioProcessorValueTreeState& apvts, juce::MidiKeyboardState& keyboardState);
    ~SynthView() override = default;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;

private:
    std::unique_ptr<OscASection> oscA;
    std::unique_ptr<OscBSection> oscB;
    std::unique_ptr<NoiseSection> noise;
    std::unique_ptr<FilterSectionUI> filter;
    
    std::unique_ptr<ModulationSectionUI> modulation;
    std::unique_ptr<GlobalEnvSection> globalEnv;
    std::unique_ptr<SoftClipperSection> softClipper;
    std::unique_ptr<ReverbSection> reverb;

    juce::MidiKeyboardComponent midiKeyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthView)
};

} // namespace genesynth
