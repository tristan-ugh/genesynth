#pragma once
#include "Framework/aBlock/aBlock.h"
#include "Parameters/StaticParameter.h"

namespace genesynth {

class SoftClipper : public aBlock {
public:
    SoftClipper(StaticParameter* threshold);
    
    void prepare(const juce::dsp::ProcessSpec&) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    StaticParameter* THRESHOLD;
};

} // namespace genesynth
