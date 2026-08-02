#pragma once
#include "Framework/aBlock/aBlock.h"
#include "Parameters/ModulableParameter.h"
#include "Parameters/StaticParameter.h"
#include <JuceHeader.h>

namespace genesynth {

class GlobalAmp : public aBlock {
public:
    GlobalAmp(StaticParameter* attack, StaticParameter* decay, StaticParameter* sustain, StaticParameter* release, StaticParameter* gate);
        
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;
    
    void noteOn();
    void noteOff();
    bool isActive() const;

private:
    StaticParameter* A;
    StaticParameter* D;
    StaticParameter* S;
    StaticParameter* R;
    
    StaticParameter* GATE;
    juce::ADSR adsr;
    juce::ADSR::Parameters lastParams;
    bool wasGateHigh = false;
};

} // namespace genesynth
