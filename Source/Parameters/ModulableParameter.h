#pragma once
#include "SynthParameter.h"
#include "StaticParameter.h"
#include <atomic>
#include <algorithm>

namespace genesynth {

class ModulableParameter : public SynthParameter {
public:
    ModulableParameter(ParamID id, const std::string& name, float defaultVal);
    void setBaseValue(float val);
    float getFinalValue(float modSignal) const;
    float getRawValue() const;
    float getModAmount() const;

    void setModAmountParam(StaticParameter* param);

private:
    std::atomic<float> currentValue;
    StaticParameter* modAmountParam = nullptr;
};

} // namespace genesynth
