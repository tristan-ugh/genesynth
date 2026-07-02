#include "StaticParameter.h"

namespace genesynth {

StaticParameter::StaticParameter(ParamID id, const std::string& name, float defaultVal)
    : SynthParameter(id, name, defaultVal), value(defaultVal) {}

void StaticParameter::setValue(float val) { value.store(val, std::memory_order_relaxed); }
float StaticParameter::getValue() const { return value.load(std::memory_order_relaxed); }

} // namespace genesynth
