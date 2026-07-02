#pragma once
#include "Framework/aBlock/aBlock.h"
#include "Parameters/StaticParameter.h"
#include "Parameters/ModulableParameter.h"
#include <JuceHeader.h>

namespace genesynth {

class LadderFilter : public aBlock {
public:
    LadderFilter(StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix, const float* macroSignal);
    
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    const float* macroSignal;
    StaticParameter* TYPE;
    StaticParameter* RESONANCE;
    ModulableParameter* CUTOFF;
    ModulableParameter* MIX;
    juce::dsp::StateVariableTPTFilter<float> filter;
};

} // namespace genesynth
