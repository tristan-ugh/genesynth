#include "aOsc.h"

namespace genesynth {

namespace {
    inline float poly_blep(float t, float dt) {
        float abs_dt = std::abs(dt);
        if (abs_dt < 1e-6f) return 0.0f;
        
        float residual = 0.0f;
        if (t < abs_dt) {
            t /= abs_dt;
            residual = t + t - t * t - 1.0f;
        } else if (t > 1.0f - abs_dt) {
            t = (t - 1.0f) / abs_dt;
            residual = t * t + t + t + 1.0f;
        }
        return (dt > 0.0f) ? residual : -residual;
    }
}

aOsc::aOsc(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, const float* macroSignal)
    : aGenerator(vol, freq), wave(wave), ratio(ratio), macroSignal(macroSignal) {}
    
void aOsc::prepare(const juce::dsp::ProcessSpec& spec) {
    sampleRate = spec.sampleRate;
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.0); // We will smooth manually
    smoothVol.reset(spec.sampleRate, 0.02); // 20ms smoothing
    smoothFreq.reset(spec.sampleRate, 0.02);
}

void aOsc::reset() {
    currentPhase = 0.0f;
    gain.reset();
}

void aOsc::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    float macro = macroSignal ? *macroSignal : 0.0f;
    
    float targetFreqBase = FREQ ? FREQ->getRawValue() : 0.0f;
    smoothFreq.setTargetValue(targetFreqBase);
    float baseFreqMod = smoothFreq.getNextValue();
    float fModAmt = FREQ ? FREQ->getModAmount() : 0.0f;
    float detuneMod = baseFreqMod + macro * fModAmt;
    float detuneMult = std::pow(2.0f, detuneMod);
    
    float baseFreq = internalVoiceFreq * ratio->getValue();
    float detunedFreq = baseFreq * detuneMult;
    
    float fm = getFrequencyModulation(macro);
    
    // Linear Through-Zero FM
    // The FM deviation is based on the base frequency so that the 'FREQ' detune
    // only offsets the final pitch without scaling the FM depth!
    float modulatedFreq = detunedFreq + baseFreq * fm;
    
    // Clamp to nyquist on both sides (allowing TZFM)
    float maxFreq = (float)(sampleRate * 0.49);
    modulatedFreq = std::clamp(modulatedFreq, -maxFreq, maxFreq);
    
    float dt = modulatedFreq / (float)sampleRate;
    currentPhase += dt;
    if (currentPhase >= 1.0f) currentPhase -= std::floor(currentPhase);
    else if (currentPhase < 0.0f) currentPhase -= std::floor(currentPhase);
    
    float t = currentPhase;
    
    float wf = std::clamp(wave ? wave->getValue() : 0.0f, 0.0f, 1.0f) * 3.0f;
    int w1 = std::clamp((int)std::floor(wf), 0, 3);
    int w2 = std::clamp(w1 + 1, 0, 3);
    float mix = wf - w1;
    
    auto getWaveShape = [t, dt](int type) -> float {
        if (type == 0) { // Sine
            return std::sin(t * juce::MathConstants<float>::twoPi);
        } else if (type == 1) { // Triangle (Sine-aligned)
            float s = t + 0.25f;
            if (s >= 1.0f) s -= 1.0f;
            return 1.0f - 4.0f * std::abs(s - 0.5f);
        } else if (type == 2) { // Square (Sine-aligned fundamental)
            float naive = t < 0.5f ? 1.0f : -1.0f;
            float shift = t + 0.5f;
            if (shift >= 1.0f) shift -= 1.0f;
            return naive + poly_blep(t, dt) - poly_blep(shift, dt);
        } else { // Saw (Downward, Sine-aligned)
            float naive = 1.0f - 2.0f * t;
            return naive + poly_blep(t, dt);
        }
    };
    
    float out = getWaveShape(w1);
    if (mix > 0.001f && w1 != w2) {
        out = out * (1.0f - mix) + getWaveShape(w2) * mix;
    }
    
    float targetVolBase = VOL ? VOL->getRawValue() : 1.0f;
    smoothVol.setTargetValue(targetVolBase);
    float baseVol = smoothVol.getNextValue();
    float vModAmt = VOL ? VOL->getModAmount() : 0.0f;
    float volMod = std::clamp(baseVol + macro * vModAmt, 0.0f, 1.0f);
    
    gain.setGainLinear(volMod);
    
    // Process block manually (since it's exactly 1 sample!)
    auto& outBlock = context.getOutputBlock();
    outBlock.getChannelPointer(0)[0] = out;
    
    lastRawSample = out;
    gain.process(context);
}

} // namespace genesynth
