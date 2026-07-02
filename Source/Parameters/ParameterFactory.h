#pragma once
#include "StaticParameter.h"
#include "ModulableParameter.h"
#include <memory>

namespace genesynth {

class ParameterFactory {
public:
    static std::unique_ptr<SynthParameter> create(ParamID id, const std::string& name, float defaultVal, bool isModulable);
};

} // namespace genesynth
