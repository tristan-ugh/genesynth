#include "FilterSection.h"

namespace genesynth {

FilterSection::FilterSection(StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix, StaticParameter* clipperThreshold, const float* macroSignal)
    : filter(filterType, filterRes, filterCutoff, filterMix, macroSignal),
      clipper(clipperThreshold)
{
    this->addBlock(&filter);
    this->addBlock(&clipper);
}

} // namespace genesynth
