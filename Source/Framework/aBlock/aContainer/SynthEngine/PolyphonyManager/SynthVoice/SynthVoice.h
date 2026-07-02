#pragma once
#include <JuceHeader.h>
#include "Framework/aBlock/aContainer/SeriesContainer.h"
#include "Parameters/ModulableParameter.h"
#include "Parameters/StaticParameter.h"

#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/ExcitationSection.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/FilterSection.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/GlobalAmp.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/SynthSound.h"

namespace genesynth {

class SynthVoice : public juce::SynthesiserVoice, public SeriesContainer {
public:
    SynthVoice(
        ModulableParameter* oscAVol, ModulableParameter* oscAFreq, StaticParameter* oscAWave, StaticParameter* oscARatio, ModulableParameter* oscAFm,
        ModulableParameter* oscBVol, ModulableParameter* oscBFreq, StaticParameter* oscBWave, StaticParameter* oscBRatio,
        ModulableParameter* noiseVol, ModulableParameter* noiseFreq, StaticParameter* noiseType,
        StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix,
        StaticParameter* clipperThreshold,
        StaticParameter* envA, StaticParameter* envD, StaticParameter* envS, StaticParameter* envR, StaticParameter* gate,
        StaticParameter* envModA, StaticParameter* envModD, StaticParameter* envModS, StaticParameter* envModR,
        StaticParameter* lfoAmt, StaticParameter* lfoFreq
    );
    virtual ~SynthVoice() = default;

    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    
    void process(const juce::dsp::ProcessContextReplacing<float>& context) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);

private:
    juce::ADSR modEnv;
    juce::ADSR::Parameters modEnvParams;
    float lfoPhase = 0.0f;
    float currentMacroSignal = 0.0f;

    StaticParameter* envModA;
    StaticParameter* envModD;
    StaticParameter* envModS;
    StaticParameter* envModR;
    StaticParameter* lfoAmt;
    StaticParameter* lfoFreq;

    ExcitationSection excitationSection;
    FilterSection filterSection;
    GlobalAmp ampEnv;

    float currentBaseFrequency = 440.0f;
    juce::AudioBuffer<float> voiceBuffer;
    bool isPrepared = false;
};

} // namespace genesynth
