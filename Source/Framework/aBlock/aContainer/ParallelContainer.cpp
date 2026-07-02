#include "ParallelContainer.h"

namespace genesynth {

void ParallelContainer::prepare(const juce::dsp::ProcessSpec& spec) {
    aContainer::prepare(spec);
    tempBuffer.setSize(spec.numChannels, spec.maximumBlockSize);
}

void ParallelContainer::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();
    
    outputBlock.clear();
    
    for (auto* block : childBlocks) {
        juce::dsp::AudioBlock<float> tempAudioBlock(tempBuffer);
        auto subBlock = tempAudioBlock.getSubBlock(0, inputBlock.getNumSamples());
        auto tempContext = juce::dsp::ProcessContextReplacing<float>(subBlock);
        
        subBlock.copyFrom(inputBlock);
        block->process(tempContext);
        outputBlock.add(subBlock);
    }
}

} // namespace genesynth
