#include "NoiseGenerator.h"

namespace genesynth {

NoiseGenerator::NoiseGenerator(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* type, const float* macroSignal)
    : aGenerator(vol, nullptr), type(type), macroSignal(macroSignal) {}

void NoiseGenerator::prepare(const juce::dsp::ProcessSpec& spec) {
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.0);
    smoothVol.reset(spec.sampleRate, 0.02);
}

void NoiseGenerator::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto&& outBlock = context.getOutputBlock();
    for (int ch = 0; ch < outBlock.getNumChannels(); ++ch) {
        auto* ptr = outBlock.getChannelPointer(ch);
        for (int i = 0; i < outBlock.getNumSamples(); ++i) {
            ptr[i] = random.nextFloat() * 2.0f - 1.0f;
        }
    }
    
    float cutoff = juce::jmap(type->getValue(), 0.0f, 1.0f, 20.0f, 20000.0f);
    filter.setCutoffFrequency(cutoff);
    filter.process(context);
    
    float macro = macroSignal ? *macroSignal : 0.0f;
    float targetVolBase = VOL ? VOL->getRawValue() : 1.0f;
    smoothVol.setTargetValue(targetVolBase);
    float baseVol = smoothVol.getNextValue();
    float vModAmt = VOL ? VOL->getModAmount() : 0.0f;
    float volMod = std::clamp(baseVol + macro * vModAmt, 0.0f, 1.0f);
    
    gain.setGainLinear(volMod);
    gain.process(context);
}

} // namespace genesynth
