#include "GlobalAmp.h"

namespace genesynth {

GlobalAmp::GlobalAmp(StaticParameter* attack, StaticParameter* decay, StaticParameter* sustain, StaticParameter* release, StaticParameter* gate)
    : A(attack), D(decay), S(sustain), R(release), GATE(gate) {}

void GlobalAmp::prepare(const juce::dsp::ProcessSpec& spec) {
    adsr.setSampleRate(spec.sampleRate);
}

void GlobalAmp::reset() {
    adsr.reset();
}

void GlobalAmp::noteOn() {
    adsr.noteOn();
}

void GlobalAmp::noteOff() {
    adsr.noteOff();
}

bool GlobalAmp::isActive() const {
    return adsr.isActive();
}

void GlobalAmp::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    juce::ADSR::Parameters params;
    params.attack = juce::jmap(A->getValue(), 0.0f, 1.0f, 0.001f, 5.0f);
    params.decay = juce::jmap(D->getValue(), 0.0f, 1.0f, 0.001f, 5.0f);
    params.sustain = S->getValue();
    params.release = juce::jmap(R->getValue(), 0.0f, 1.0f, 0.001f, 5.0f);
    adsr.setParameters(params);
    
    bool isGateHigh = (GATE->getValue() >= 0.5f);
    if (isGateHigh != wasGateHigh) {
        wasGateHigh = isGateHigh;
        if (isGateHigh) adsr.noteOn();
        else adsr.noteOff();
    }
    
    auto&& block = context.getOutputBlock();
    for (int sample = 0; sample < block.getNumSamples(); ++sample) {
        float envVal = adsr.getNextSample();
        for (int ch = 0; ch < block.getNumChannels(); ++ch) {
            block.getChannelPointer(ch)[sample] *= envVal;
        }
    }
}

} // namespace genesynth
