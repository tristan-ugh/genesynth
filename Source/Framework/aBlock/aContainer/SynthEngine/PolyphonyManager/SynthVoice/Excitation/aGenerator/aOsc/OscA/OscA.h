#pragma once
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/aOsc.h"

namespace genesynth {

class OscA : public aOsc {
public:
    OscA(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, ModulableParameter* fm, const float* macroSignal);

    void setFmInput(float fm) { fmInput = fm; }
    
    float getFrequencyModulation(float macro) override {
        if (!FM) return 0.0f;
        float fmBase = FM->getRawValue();
        float fmMcro = FM->getModAmount();
        // Return linear fm amount. fmInput is usually between -1.0 and 1.0.
        // We allow up to +/- 4x base frequency modulation.
        return (fmBase + macro * fmMcro) * fmInput * 4.0f;
    }

    ModulableParameter* FM;
    float fmInput = 0.0f;
};

} // namespace genesynth
