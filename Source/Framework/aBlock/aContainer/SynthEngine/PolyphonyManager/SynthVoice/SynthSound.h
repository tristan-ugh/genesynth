#pragma once
#include <JuceHeader.h>

namespace genesynth {
class SynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};
} // namespace genesynth
