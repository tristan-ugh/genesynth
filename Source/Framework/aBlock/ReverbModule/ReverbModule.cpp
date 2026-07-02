#include "ReverbModule.h"

namespace genesynth {

ReverbModule::ReverbModule() {}

void ReverbModule::setParameters(StaticParameter* decay, StaticParameter* size, StaticParameter* mix) {
    DECAY = decay;
    SIZE = size;
    MIX = mix;
}

void ReverbModule::prepare(const juce::dsp::ProcessSpec& spec) {
    reverb.setSampleRate(spec.sampleRate);
    reverb.reset();
}

void ReverbModule::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    juce::Reverb::Parameters params;
    params.roomSize = SIZE ? SIZE->getValue() : 0.5f;
    params.damping = DECAY ? DECAY->getValue() : 0.5f;
    float mixVal = MIX ? MIX->getValue() : 0.0f;
    params.wetLevel = mixVal;
    params.dryLevel = 1.0f - mixVal;
    params.width = 1.0f;
    params.freezeMode = 0.0f;
    reverb.setParameters(params);

    auto&& block = context.getOutputBlock();
    if (block.getNumChannels() == 1) {
        reverb.processMono(block.getChannelPointer(0), block.getNumSamples());
    } else if (block.getNumChannels() == 2) {
        reverb.processStereo(block.getChannelPointer(0), block.getChannelPointer(1), block.getNumSamples());
    }
}

} // namespace genesynth
