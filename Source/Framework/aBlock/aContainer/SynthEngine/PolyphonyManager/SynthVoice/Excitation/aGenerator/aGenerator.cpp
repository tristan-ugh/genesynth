#include "aGenerator.h"

namespace genesynth {

aGenerator::aGenerator(ModulableParameter* vol, ModulableParameter* freq)
    : VOL(vol), FREQ(freq) {}

void aGenerator::setVoiceFrequency(float freq) { internalVoiceFreq = freq; }

} // namespace genesynth
