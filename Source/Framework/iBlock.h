#pragma once
#include <JuceHeader.h>

namespace genesynth {

class iBlock {
public:
    virtual ~iBlock() = default;
    virtual void prepare(const juce::dsp::ProcessSpec& spec) = 0;
    virtual void process(const juce::dsp::ProcessContextReplacing<float>& context) = 0;
    virtual void reset() = 0;
};

} // namespace genesynth
