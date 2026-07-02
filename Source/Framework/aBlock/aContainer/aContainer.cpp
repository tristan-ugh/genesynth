#include "aContainer.h"

namespace genesynth {

void aContainer::addBlock(iBlock* block) {
    childBlocks.push_back(block);
}

void aContainer::prepare(const juce::dsp::ProcessSpec& spec) {
    for (auto* block : childBlocks) {
        block->prepare(spec);
    }
}

void aContainer::reset() {
    for (auto* block : childBlocks) {
        block->reset();
    }
}

} // namespace genesynth
