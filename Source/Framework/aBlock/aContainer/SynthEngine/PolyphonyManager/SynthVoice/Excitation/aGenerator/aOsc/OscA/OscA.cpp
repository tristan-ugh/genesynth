#include "OscA.h"

namespace genesynth {

namespace {
    inline float poly_blep(float t, float dt) {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0f;
        } else if (t > 1.0f - dt) {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }
}

OscA::OscA(ModulableParameter* vol, ModulableParameter* freq, StaticParameter* wave, StaticParameter* ratio, ModulableParameter* fm, const float* macroSignal)
    : aOsc(vol, freq, wave, ratio, macroSignal), FM(fm) {}

} // namespace genesynth
