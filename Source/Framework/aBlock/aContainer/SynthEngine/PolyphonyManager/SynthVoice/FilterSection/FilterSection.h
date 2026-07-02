#pragma once
#include "Framework/aBlock/aContainer/SeriesContainer.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/LadderFilter/LadderFilter.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/SoftClipper/SoftClipper.h"

namespace genesynth {

class FilterSection : public SeriesContainer {
public:
    FilterSection(StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix, StaticParameter* clipperThreshold, const float* macroSignal);

private:
    LadderFilter filter;
    SoftClipper clipper;
};

} // namespace genesynth
