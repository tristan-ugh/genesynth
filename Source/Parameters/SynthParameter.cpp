#include "SynthParameter.h"

namespace genesynth {

SynthParameter::SynthParameter(ParamID id, const std::string& name, float baseValue)
    : id(id), name(name), baseValue(baseValue) {}

ParamID SynthParameter::getId() const { return id; }
const std::string& SynthParameter::getName() const { return name; }
float SynthParameter::getBaseValue() const { return baseValue; }

} // namespace genesynth
