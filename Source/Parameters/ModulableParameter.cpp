#include "ModulableParameter.h"

namespace genesynth {

ModulableParameter::ModulableParameter(ParamID id, const std::string& name, float defaultVal)
    : SynthParameter(id, name, defaultVal), currentValue(defaultVal) {}

void ModulableParameter::setBaseValue(float val) {
    baseValue = val;
    currentValue.store(val, std::memory_order_relaxed);
}

void ModulableParameter::setModAmountParam(StaticParameter* param) {
    modAmountParam = param;
}

float ModulableParameter::getFinalValue(float modSignal) const {
    float amount = modAmountParam ? modAmountParam->getValue() : 0.5f;
    float bipolarAmount = (amount * 2.0f) - 1.0f;
    float scaledMod = modSignal * bipolarAmount;
    return std::clamp(currentValue.load(std::memory_order_relaxed) + scaledMod, 0.0f, 1.0f);
}

float ModulableParameter::getRawValue() const {
    return currentValue.load(std::memory_order_relaxed);
}

float ModulableParameter::getModAmount() const {
    float amount = modAmountParam ? modAmountParam->getValue() : 0.5f;
    return (amount * 2.0f) - 1.0f;
}

} // namespace genesynth
