#include "LadderFilter.h"

namespace genesynth {

LadderFilter::LadderFilter(StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix, const float* macroSignal)
    : TYPE(filterType), RESONANCE(filterRes), CUTOFF(filterCutoff), MIX(filterMix), macroSignal(macroSignal) {
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void LadderFilter::prepare(const juce::dsp::ProcessSpec& spec) {
    filter.prepare(spec);
    mixer.prepare(spec);
    mixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
}

void LadderFilter::reset() {
    filter.reset();
    mixer.reset();
}

void LadderFilter::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    float mixVal = MIX ? MIX->getFinalValue(*macroSignal) : 1.0f;
    mixer.setWetMixProportion(mixVal);
    
    mixer.pushDrySamples(context.getInputBlock());
    
    float c = juce::jmap(CUTOFF ? CUTOFF->getFinalValue(*macroSignal) : 1.0f, 0.0f, 1.0f, 20.0f, 20000.0f);
    filter.setCutoffFrequency(c);
    
    float r = juce::jmap(RESONANCE->getValue(), 0.0f, 1.0f, 0.707f, 10.0f);
    filter.setResonance(r);
    
    float t = TYPE->getValue();
    if (t < 0.33f) filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    else if (t < 0.66f) filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    else filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    
    filter.process(context);
    
    mixer.mixWetSamples(context.getOutputBlock());
}

} // namespace genesynth
