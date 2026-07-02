#pragma once
#include "Framework/aBlock/aBlock.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/OscA/OscA.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/OscB/OscB.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/Noise/NoiseGenerator.h"

namespace genesynth {

class ExcitationSection : public aBlock {
public:
    ExcitationSection(
        ModulableParameter* oscAVol, ModulableParameter* oscAFreq, StaticParameter* oscAWave, StaticParameter* oscARatio, ModulableParameter* oscAFm,
        ModulableParameter* oscBVol, ModulableParameter* oscBFreq, StaticParameter* oscBWave, StaticParameter* oscBRatio,
        ModulableParameter* noiseVol, ModulableParameter* noiseFreq, StaticParameter* noiseType, const float* macroSignal
    );
    void setVoiceFrequency(float freq);

    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

private:
    OscA oscA;
    OscB oscB;
    NoiseGenerator noise;
    
    juce::AudioBuffer<float> tempBuffer;
};

} // namespace genesynth
