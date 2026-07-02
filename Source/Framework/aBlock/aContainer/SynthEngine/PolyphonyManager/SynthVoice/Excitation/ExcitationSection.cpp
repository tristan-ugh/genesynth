#include "ExcitationSection.h"

namespace genesynth {

ExcitationSection::ExcitationSection(
        ModulableParameter* oscAVol, ModulableParameter* oscAFreq, StaticParameter* oscAWave, StaticParameter* oscARatio, ModulableParameter* oscAFm,
        ModulableParameter* oscBVol, ModulableParameter* oscBFreq, StaticParameter* oscBWave, StaticParameter* oscBRatio,
        ModulableParameter* noiseVol, ModulableParameter* noiseFreq, StaticParameter* noiseType, const float* macroSignal
    ) : oscA(oscAVol, oscAFreq, oscAWave, oscARatio, oscAFm, macroSignal),
        oscB(oscBVol, oscBFreq, oscBWave, oscBRatio, macroSignal),
        noise(noiseVol, noiseFreq, noiseType, macroSignal)
    {}

void ExcitationSection::setVoiceFrequency(float freq) {
    oscA.setVoiceFrequency(freq);
    oscB.setVoiceFrequency(freq);
    noise.setVoiceFrequency(freq);
}

void ExcitationSection::prepare(const juce::dsp::ProcessSpec& spec) {
    oscA.prepare(spec);
    oscB.prepare(spec);
    noise.prepare(spec);
    tempBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void ExcitationSection::reset() {
    oscA.reset();
    oscB.reset();
    noise.reset();
}

void ExcitationSection::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    outputBlock.clear();
    
    juce::dsp::AudioBlock<float> tempAudioBlock(tempBuffer);
    auto subBlock = tempAudioBlock.getSubBlock(0, inputBlock.getNumSamples());
    auto tempContext = juce::dsp::ProcessContextReplacing<float>(subBlock);
    
    // Process OscB
    subBlock.copyFrom(inputBlock);
    oscB.process(tempContext);
    
    // Extract FM signal from OscB raw output (bypassing OscB volume)
    float fmSignal = oscB.getLastRawSample();
    oscA.setFmInput(fmSignal);
    
    outputBlock.add(subBlock); // Mix OscB to output
    
    // Process OscA
    subBlock.copyFrom(inputBlock);
    oscA.process(tempContext);
    outputBlock.add(subBlock); // Mix OscA to output
    
    // Process Noise
    subBlock.copyFrom(inputBlock);
    noise.process(tempContext);
    outputBlock.add(subBlock); // Mix Noise to output
}

} // namespace genesynth
