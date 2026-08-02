#include "SynthVoice.h"

namespace genesynth {

SynthVoice::SynthVoice(
    ModulableParameter* oscAVol, ModulableParameter* oscAFreq, StaticParameter* oscAWave, StaticParameter* oscARatio, ModulableParameter* oscAFm,
    ModulableParameter* oscBVol, ModulableParameter* oscBFreq, StaticParameter* oscBWave, StaticParameter* oscBRatio,
    ModulableParameter* noiseVol, ModulableParameter* noiseFreq, StaticParameter* noiseType,
    StaticParameter* filterType, StaticParameter* filterRes, ModulableParameter* filterCutoff, ModulableParameter* filterMix,
    StaticParameter* clipperThreshold,
    StaticParameter* envA, StaticParameter* envD, StaticParameter* envS, StaticParameter* envR, StaticParameter* gate,
    StaticParameter* envModA, StaticParameter* envModD, StaticParameter* envModS, StaticParameter* envModR,
    StaticParameter* lfoAmt, StaticParameter* lfoFreq
) : 
    excitationSection(oscAVol, oscAFreq, oscAWave, oscARatio, oscAFm,
                      oscBVol, oscBFreq, oscBWave, oscBRatio,
                      noiseVol, noiseFreq, noiseType, &currentMacroSignal),
    filterSection(filterType, filterRes, filterCutoff, filterMix, clipperThreshold, &currentMacroSignal),
    ampEnv(envA, envD, envS, envR, gate),
    envModA(envModA), envModD(envModD), envModS(envModS), envModR(envModR), lfoAmt(lfoAmt), lfoFreq(lfoFreq)
{
    modEnv.setSampleRate(44100.0);
    this->addBlock(&excitationSection);
    this->addBlock(&filterSection);
    this->addBlock(&ampEnv);
}

bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound) {
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels) {
    modEnv.setSampleRate(sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    this->prepare(spec);

    voiceBuffer.setSize(1, samplesPerBlock, false, false, true);
    isPrepared = true;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) {
    modEnvParams.attack = envModA->getValue() * 2.0f;
    modEnvParams.decay = envModD->getValue() * 2.0f;
    modEnvParams.sustain = envModS->getValue();
    modEnvParams.release = envModR->getValue() * 2.0f;
    modEnv.setParameters(modEnvParams);
    modEnv.noteOn();
    lfoPhase = 0.0f;

    if (!isPrepared) return;
    currentBaseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    excitationSection.setVoiceFrequency(currentBaseFrequency);
    
    ampEnv.noteOn();
}

void SynthVoice::stopNote(float velocity, bool allowTailOff) {
    modEnv.noteOff();

    ampEnv.noteOff();
}

void SynthVoice::pitchWheelMoved(int /*newPitchWheelValue*/) {}
void SynthVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/) {}

void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) {
    if (!isPrepared || !isVoiceActive()) return;

    voiceBuffer.setSize(1, numSamples, false, false, true);
    voiceBuffer.clear();

    juce::dsp::AudioBlock<float> audioBlock(voiceBuffer);
    juce::dsp::ProcessContextReplacing<float> context(audioBlock);

    this->process(context);

    for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch) {
        outputBuffer.addFrom(ch, startSample, voiceBuffer, 0, 0, numSamples);
    }
    
    if (!ampEnv.isActive()) {
        clearCurrentNote();
    }
}
void SynthVoice::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto&& outBlock = context.getOutputBlock();
    int numSamples = (int)outBlock.getNumSamples();

    // Boucle d'Audio-Rate Modulation : on traite 1 sample à la fois !
    for (int i = 0; i < numSamples; ++i) {
        // Récupération des valeurs LFO (contrôle lissé)
        float hz = juce::jmap(lfoFreq->getValue(), 0.0f, 1.0f, 0.1f, 20.0f);
        float amt = lfoAmt->getValue();
        float phaseInc = juce::MathConstants<float>::twoPi * hz / getSampleRate();
        
        // 1. Mise à jour de l'enveloppe avec courbe n^2 (exponentielle/quadratique)
        float envVal = modEnv.getNextSample();
        envVal = envVal * envVal;
        
        // 2. Mise à jour du LFO
        float lfoVal = std::sin(lfoPhase) * amt;
        lfoPhase += phaseInc;
        if (lfoPhase >= juce::MathConstants<float>::twoPi) {
            lfoPhase -= juce::MathConstants<float>::twoPi;
        }
        
        // 3. Mise à jour du macro-signal pour CE sample
        currentMacroSignal = envVal + lfoVal;
        
        // 4. Traitement du moteur DSP complet pour CE sample
        juce::dsp::AudioBlock<float> singleSampleBlock = outBlock.getSubBlock(i, 1);
        juce::dsp::ProcessContextReplacing<float> singleSampleContext(singleSampleBlock);
        SeriesContainer::process(singleSampleContext);
    }
}

} // namespace genesynth
