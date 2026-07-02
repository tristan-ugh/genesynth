#include "SoftClipper.h"
#include <cmath>

namespace genesynth {

SoftClipper::SoftClipper(StaticParameter* threshold) : THRESHOLD(threshold) {}

void SoftClipper::prepare(const juce::dsp::ProcessSpec&) {}

void SoftClipper::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto&& block = context.getOutputBlock();
    float thresh = THRESHOLD->getValue();
    
    for (int ch = 0; ch < block.getNumChannels(); ++ch) {
        auto* ptr = block.getChannelPointer(ch);
        for (int i = 0; i < block.getNumSamples(); ++i) {
            ptr[i] = std::tanh(ptr[i] * thresh);
        }
    }
}

} // namespace genesynth
