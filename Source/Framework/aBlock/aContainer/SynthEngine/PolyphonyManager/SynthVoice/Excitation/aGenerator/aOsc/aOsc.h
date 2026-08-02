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

    virtual float getFrequencyModulation(float /*macro*/) { return 0.0f; }

protected:
    const float* macroSignal;
    StaticParameter* wave;
    StaticParameter* ratio;
    double sampleRate = 44100.0;
    float currentPhase = 0.0f;
    
    juce::dsp::Gain<float> gain;
    
    juce::SmoothedValue<float> smoothVol;
    juce::SmoothedValue<float> smoothFreq;
    float lastRawSample = 0.0f;
};

} // namespace genesynth
