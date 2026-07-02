#include "OscB.h"

namespace genesynth {

OscB::OscB(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, const float* macroSignal)
    : aOsc(vol, freq, wave, ratio, macroSignal) {}

} // namespace genesynth
