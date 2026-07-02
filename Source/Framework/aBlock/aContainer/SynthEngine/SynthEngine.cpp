#include "SynthEngine.h"

namespace genesynth {

SynthEngine::SynthEngine() {
    this->addBlock(&polyphony);
    this->addBlock(&reverb);
}

PolyphonyManager& SynthEngine::getPolyphonyManager() { return polyphony; }
ReverbModule& SynthEngine::getReverb() { return reverb; }

} // namespace genesynth
