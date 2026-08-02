#include "NoiseGenerator.h"

namespace genesynth {

NoiseGenerator::NoiseGenerator(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* type, const float* macroSignal)
    : aGenerator(vol, freq), type(type), macroSignal(macroSignal) {}

void NoiseGenerator::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    filter.prepare(spec);
    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.0);
    smoothVol.reset(spec.sampleRate, 0.02);
    smoothFreq.reset(spec.sampleRate, 0.02);
}

void NoiseGenerator::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    float macro = macroSignal ? *macroSignal : 0.0f;
    
    // DJ Filter logic (freq parameter)
    float targetFreqBase = FREQ ? FREQ->getRawValue() : 0.5f; // Middle is 0.5
    smoothFreq.setTargetValue(targetFreqBase);
    float baseFreq = smoothFreq.getNextValue();
    float fModAmt = FREQ ? FREQ->getModAmount() : 0.0f;
    float freqMod = std::clamp(baseFreq + macro * fModAmt, 0.0f, 1.0f);
    
    if (freqMod < 0.49f) {
        // Lowpass mode (aggressive)
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        // Map 0.0 -> 20Hz, 0.49 -> 20000Hz (exponential)
        float normalized = freqMod / 0.49f;
        float cutoff = 20.0f * std::pow(1000.0f, normalized); // 20 * 1000 = 20000
        filter.setCutoffFrequency(std::clamp(cutoff, 20.0f, 20000.0f));
    } else if (freqMod > 0.51f) {
        // Highpass mode (aggressive)
        filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        // Map 0.51 -> 20Hz, 1.0 -> 20000Hz (exponential)
        float normalized = (freqMod - 0.51f) / 0.49f;
        float cutoff = 20.0f * std::pow(1000.0f, normalized); // 20 * 1000 = 20000
        filter.setCutoffFrequency(std::clamp(cutoff, 20.0f, 20000.0f));
    } else {
        // Bypass filter in the middle
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); // doesn't matter, we won't process it or we set it to allpass
        // Actually, easiest way to bypass is to just not call filter.process() or set it to allpass?
        // Let's set it to LP at 20000Hz
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        filter.setCutoffFrequency(20000.0f);
    }
    
    // Noise color (type parameter)
    float t = std::clamp(type->getValue(), 0.0f, 1.0f);
    float alpha = 0.05f + 0.95f * t; // 1-pole lowpass coeff matches visualizer
    
    auto&& outBlock = context.getOutputBlock();
    for (int ch = 0; ch < outBlock.getNumChannels(); ++ch) {
        auto* ptr = outBlock.getChannelPointer(ch);
        float tempZ = z;
        for (int i = 0; i < outBlock.getNumSamples(); ++i) {
            float randVal = random.nextFloat() * 2.0f - 1.0f;
            tempZ = tempZ + alpha * (randVal - tempZ);
            
            // Gain compensation for dark colors (Brown noise is very quiet)
            float displayZ = tempZ * (1.0f + (1.0f - alpha) * 2.0f);
            ptr[i] = std::clamp(displayZ, -1.0f, 1.0f);
        }
        if (ch == outBlock.getNumChannels() - 1) z = tempZ;
    }
    
    if (freqMod < 0.49f || freqMod > 0.51f) {
        filter.process(context);
    }
    
    float targetVolBase = VOL ? VOL->getRawValue() : 1.0f;
    smoothVol.setTargetValue(targetVolBase);
    float baseVol = smoothVol.getNextValue();
    float vModAmt = VOL ? VOL->getModAmount() : 0.0f;
    float volMod = std::clamp(baseVol + macro * vModAmt, 0.0f, 1.0f);
    
    gain.setGainLinear(volMod);
    gain.process(context);
}

} // namespace genesynth
