#include "aOsc.h"

namespace genesynth {

aOsc::aOsc(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, const float* macroSignal)
    : aGenerator(vol, freq), wave(wave), ratio(ratio), macroSignal(macroSignal) {}
    
void aOsc::prepare(const juce::dsp::ProcessSpec& spec) {
    osc.prepare(spec);
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.0); // We will smooth manually
    smoothVol.reset(spec.sampleRate, 0.02); // 20ms smoothing
    smoothFreq.reset(spec.sampleRate, 0.02);
}

void aOsc::reset() {
    osc.reset();
    gain.reset();
}

void aOsc::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    if (wave) {
        float v = wave->getValue();
        int newWave = 0;
        if (v < 0.25f) newWave = 0;
        else if (v < 0.5f) newWave = 1;
        else if (v < 0.75f) newWave = 2;
        else newWave = 3;
        
        if (newWave != currentWaveform) {
            currentWaveform = newWave;
            if (newWave == 0) { // Sine
                osc.initialise([](float x) { return std::sin(x); }, 2048);
            } else if (newWave == 1) { // Triangle
                osc.initialise([](float x) { return (2.0f / juce::MathConstants<float>::pi) * std::asin(std::sin(x)); }, 2048);
            } else if (newWave == 2) { // Square
                osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; }, 2048);
            } else { // Saw
                osc.initialise([](float x) { return x / juce::MathConstants<float>::pi; }, 2048);
            }
        }
    }
    
    float macro = macroSignal ? *macroSignal : 0.0f;
    
    float targetFreqBase = FREQ ? FREQ->getRawValue() : 0.0f;
    smoothFreq.setTargetValue(targetFreqBase);
    float baseFreqMod = smoothFreq.getNextValue();
    float fModAmt = FREQ ? FREQ->getModAmount() : 0.0f;
    float detuneMod = baseFreqMod + macro * fModAmt;
    float detuneMult = std::pow(2.0f, detuneMod);
    
    float finalFreq = internalVoiceFreq * ratio->getValue() * detuneMult;
    osc.setFrequency(finalFreq, true);
    
    float targetVolBase = VOL ? VOL->getRawValue() : 1.0f;
    smoothVol.setTargetValue(targetVolBase);
    float baseVol = smoothVol.getNextValue();
    float vModAmt = VOL ? VOL->getModAmount() : 0.0f;
    float volMod = std::clamp(baseVol + macro * vModAmt, 0.0f, 1.0f);
    
    gain.setGainLinear(volMod);
    
    osc.process(context);
    lastRawSample = context.getOutputBlock().getSample(0, 0);
    gain.process(context);
}

} // namespace genesynth
