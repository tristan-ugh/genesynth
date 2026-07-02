#pragma once
#include "Framework/iBlock.h"
#include <JuceHeader.h>

namespace genesynth {

class PolyphonyManager : public iBlock {
public:
    PolyphonyManager();
    ~PolyphonyManager() override = default;

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    void reset() override;

    void setMidiBuffer(juce::MidiBuffer* midi);
    juce::Synthesiser& getJuceSynth();

private:
    juce::Synthesiser synth;
    juce::MidiBuffer* currentMidi = nullptr;
};

} // namespace genesynth
