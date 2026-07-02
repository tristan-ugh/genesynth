#include "SeriesContainer.h"

namespace genesynth {

void SeriesContainer::process(const juce::dsp::ProcessContextReplacing<float>& context) {
    for (auto* block : childBlocks) {
        block->process(context);
    }
}

} // namespace genesynth
