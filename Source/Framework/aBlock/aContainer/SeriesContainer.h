#pragma once
#include "aContainer.h"

namespace genesynth {

class SeriesContainer : public aContainer {
public:
    virtual ~SeriesContainer() = default;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
};

} // namespace genesynth
