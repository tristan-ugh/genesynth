#include "ParameterFactory.h"

namespace genesynth {

std::unique_ptr<SynthParameter> ParameterFactory::create(ParamID id, const std::string& name, float defaultVal, bool isModulable) {
    if (isModulable) {
        return std::make_unique<ModulableParameter>(id, name, defaultVal);
    } else {
        return std::make_unique<StaticParameter>(id, name, defaultVal);
    }
}

} // namespace genesynth
