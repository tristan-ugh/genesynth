#pragma once
#include "Framework/aBlock/aBlock.h"
#include <vector>

namespace genesynth {

class aContainer : public aBlock {
public:
    virtual ~aContainer() = default;
    void addBlock(iBlock* block);
    void prepare(const juce::dsp::ProcessSpec& spec) override;
    void reset() override;

protected:
    std::vector<iBlock*> childBlocks;
};

} // namespace genesynth
