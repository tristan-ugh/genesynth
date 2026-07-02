#pragma once
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aGenerator.h"
#include "Parameters/StaticParameter.h"
#include <JuceHeader.h>

namespace genesynth {

class NoiseGenerator : public aGenerator {
public:
    NoiseGenerator(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* type, const float* macroSignal);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    const float* macroSignal;
    StaticParameter* type;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Gain<float> gain;
    juce::Random random;
    
    juce::SmoothedValue<float> smoothVol;
};

} // namespace genesynth
