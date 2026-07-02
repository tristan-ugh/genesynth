#pragma once
#include "SynthParameter.h"
#include <atomic>

namespace genesynth {

class StaticParameter : public SynthParameter {
public:
    StaticParameter(ParamID id, const std::string& name, float defaultVal);
    void setValue(float val);
    float getValue() const;

private:
    std::atomic<float> value;
};

} // namespace genesynth
