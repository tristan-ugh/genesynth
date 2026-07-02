#include "LadderFilter.h"

namespace genesynth {

LadderFilter::LadderFilter(StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix, const float* macroSignal)
    : TYPE(filterType), RESONANCE(filterRes), CUTOFF(filterCutoff), MIX(filterMix), macroSignal(macroSignal) {
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void LadderFilter::prepare(const juce::dsp::ProcessSpec& spec) {
    filter.prepare(spec);
}

void LadderFilter::reset() {
    filter.reset();
}

void LadderFilter::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    float c = juce::jmap(CUTOFF ? CUTOFF->getFinalValue(*macroSignal) : 1.0f, 0.0f, 1.0f, 20.0f, 20000.0f);
    filter.setCutoffFrequency(c);
    
    float r = juce::jmap(RESONANCE->getValue(), 0.0f, 1.0f, 0.707f, 10.0f);
    filter.setResonance(r);
    
    filter.process(context);
}

} // namespace genesynth
