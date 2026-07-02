#pragma once
#include "Framework/aBlock/aBlock.h"
#include "Parameters/ModulableParameter.h"

namespace genesynth {

class aGenerator : public aBlock {
public:
    aGenerator(ModulableParameter* vol, ModulableParameter* freq);
    void setVoiceFrequency(float freq);

protected:
    ModulableParameter* VOL;
    ModulableParameter* FREQ;
    float internalVoiceFreq = 440.0f;
};

} // namespace genesynth
