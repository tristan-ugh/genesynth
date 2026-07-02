#pragma once
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/aOsc.h"

namespace genesynth {

class OscB : public aOsc {
public:
    OscB(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, const float* macroSignal);
};

} // namespace genesynth
