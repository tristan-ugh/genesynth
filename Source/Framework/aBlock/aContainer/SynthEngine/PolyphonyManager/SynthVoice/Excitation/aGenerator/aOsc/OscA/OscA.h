#pragma once
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/aOsc.h"

namespace genesynth {

class OscA : public aOsc {
public:
    OscA(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, ModulableParameter* fm, const float* macroSignal);

    void setFmInput(float fm) { fmInput = fm; }
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    ModulableParameter* FM;
    float fmInput = 0.0f;
};

} // namespace genesynth
