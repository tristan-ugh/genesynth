#pragma once
#include <JuceHeader.h>
#include <string>

namespace genesynth {

enum class ParamID {
    // Générateurs (19)
    OSC_A_WAVE, OSC_A_RATIO,
    OSC_A_VOL_BASE, OSC_A_VOL_MOD,
    OSC_A_FREQ_BASE, OSC_A_FREQ_MOD,
    OSC_A_FM_BASE, OSC_A_FM_MOD,
    
    OSC_B_WAVE, OSC_B_RATIO,
    OSC_B_VOL_BASE, OSC_B_VOL_MOD,
    OSC_B_FREQ_BASE, OSC_B_FREQ_MOD,
    
    NOISE_TYPE,
    NOISE_VOL_BASE, NOISE_VOL_MOD,
    NOISE_FREQ_BASE, NOISE_FREQ_MOD,
    
    // Filtre (6)
    FILTER_TYPE, FILTER_RES,
    FILTER_CUTOFF_BASE, FILTER_CUTOFF_MOD,
    FILTER_MIX_BASE, FILTER_MIX_MOD,
    
    // Sources de Modulation (6)
    ENV_MOD_A, ENV_MOD_D, ENV_MOD_S, ENV_MOD_R,
    LFO_AMT, LFO_FREQ,
    
    // Enveloppe Globale (4)
    ENV_GLOBAL_A, ENV_GLOBAL_D, ENV_GLOBAL_S, ENV_GLOBAL_R,
    
    // Effets (+5)
    CLIPPER_VOL, CLIPPER_THRESH,
    REVERB_DECAY, REVERB_SIZE, REVERB_MIX,
    
    // System
    GATE,
    
    COUNT
};

class SynthParameter {
public:
    SynthParameter(ParamID id, const std::string& name, float baseValue);
    virtual ~SynthParameter() = default;

    ParamID getId() const;
    const std::string& getName() const;
    float getBaseValue() const;

protected:
    ParamID id;
    std::string name;
    float baseValue;
};

} // namespace genesynth
