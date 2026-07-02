#pragma once
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aGenerator.h"
#include "Parameters/StaticParameter.h"
#include <JuceHeader.h>

namespace genesynth {

class aOsc : public aGenerator {
public:
    aOsc(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, const float* macroSignal);
        
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    float getLastRawSample() const { return lastRawSample; }

protected:
    const float* macroSignal;
    StaticParameter* wave;
    StaticParameter* ratio;
    
    juce::dsp::Oscillator<float> osc { [](float x){ return std::sin(x); } };
    juce::dsp::Gain<float> gain;
    int currentWaveform = -1;
    
    juce::SmoothedValue<float> smoothVol;
    juce::SmoothedValue<float> smoothFreq;
    float lastRawSample = 0.0f;
};

} // namespace genesynth
