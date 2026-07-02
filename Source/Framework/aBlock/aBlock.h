#pragma once
#include "Framework/iBlock.h"

namespace genesynth {

class aBlock : public iBlock {
public:
    virtual ~aBlock() = default;
    void reset() override;
};

} // namespace genesynth
