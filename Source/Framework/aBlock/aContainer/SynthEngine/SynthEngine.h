#pragma once
#include "Framework/aBlock/aContainer/SeriesContainer.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/PolyphonyManager.h"
#include "Framework/aBlock/ReverbModule/ReverbModule.h"

namespace genesynth {

class SynthEngine : public SeriesContainer {
public:
    SynthEngine();
    PolyphonyManager& getPolyphonyManager();
    ReverbModule& getReverb();

private:
    PolyphonyManager polyphony;
    ReverbModule reverb;
};

} // namespace genesynth
