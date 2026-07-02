#include "OscA.h"

namespace genesynth {

OscA::OscA(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, ModulableParameter* fm, const float* macroSignal)
    : aOsc(vol, freq, wave, ratio, macroSignal), FM(fm) {}

void OscA::process(const juce::dsp::ProcessContextReplacing<float>& context) {
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
    
    // Add FM modulation from fmInput
    float fmMod = 0.0f;
    if (FM) {
        float fmBase = FM->getRawValue();
        float fmMcro = FM->getModAmount();
        fmMod = (fmBase + macro * fmMcro) * fmInput; // fmInput is the raw audio from OscB
    }
    
    // Convert fmMod to a frequency multiplier (e.g. up to +/- 2 octaves)
    float fmMult = std::pow(2.0f, fmMod * 2.0f);
    
    float finalFreq = internalVoiceFreq * ratio->getValue() * detuneMult * fmMult;
    osc.setFrequency(finalFreq, true);
    
    float targetVolBase = VOL ? VOL->getRawValue() : 1.0f;
    smoothVol.setTargetValue(targetVolBase);
    float baseVol = smoothVol.getNextValue();
    float vModAmt = VOL ? VOL->getModAmount() : 0.0f;
    float volMod = std::clamp(baseVol + macro * vModAmt, 0.0f, 1.0f);
    
    gain.setGainLinear(volMod);
    
    osc.process(context);
    gain.process(context);
}

} // namespace genesynth
