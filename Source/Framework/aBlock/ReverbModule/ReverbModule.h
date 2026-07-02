#pragma once
#include "Framework/aBlock/aBlock.h"
#include <JuceHeader.h>
#include "Parameters/StaticParameter.h"

namespace genesynth {

class ReverbModule : public aBlock {
public:
    ReverbModule();
    void setParameters(StaticParameter* decay, StaticParameter* size, StaticParameter* mix);
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    juce::Reverb reverb;
    StaticParameter* DECAY = nullptr;
    StaticParameter* SIZE = nullptr;
    StaticParameter* MIX = nullptr;
};

} // namespace genesynth
