#pragma once
#include "aContainer.h"

namespace genesynth {

class ParallelContainer : public aContainer {
public:
    virtual ~ParallelContainer() = default;
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    juce::AudioBuffer<float> tempBuffer;
};

} // namespace genesynth
