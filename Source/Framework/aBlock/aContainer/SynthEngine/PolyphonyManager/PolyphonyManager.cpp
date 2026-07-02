#include "PolyphonyManager.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/SynthSound.h"
#include "Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/SynthVoice.h"

namespace genesynth {

PolyphonyManager::PolyphonyManager() {
    synth.clearSounds();
    synth.addSound(new SynthSound());
}

void PolyphonyManager::prepare(const juce::dsp::ProcessSpec& spec) {
    synth.setCurrentPlaybackSampleRate(spec.sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i) {
        if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i))) {
            voice->prepareToPlay(spec.sampleRate, spec.maximumBlockSize, spec.numChannels);
        }
    }
}

void PolyphonyManager::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto&& block = context.getOutputBlock();
    float* ptrs[2] = { block.getChannelPointer(0), block.getNumChannels() > 1 ? block.getChannelPointer(1) : nullptr };
    juce::AudioBuffer<float> buffer(ptrs, block.getNumChannels(), block.getNumSamples());
    
    if (currentMidi && currentMidi->getNumEvents() > 0) {
        // Optional debug: std::cout << "PolyphonyManager: processing " << currentMidi->getNumEvents() << " MIDI events in renderNextBlock\n";
    }
    
    synth.renderNextBlock(buffer, *currentMidi, 0, block.getNumSamples());
}

void PolyphonyManager::reset() {
    // nothing for now
}

void PolyphonyManager::setMidiBuffer(juce::MidiBuffer* midi) {
    currentMidi = midi;
}

juce::Synthesiser& PolyphonyManager::getJuceSynth() {
    return synth;
}

} // namespace genesynth
