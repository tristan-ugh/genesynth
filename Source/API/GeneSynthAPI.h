#pragma once

#ifdef _WIN32
    #define GENESYNTH_EXPORT __declspec(dllexport)
#else
    #define GENESYNTH_EXPORT __attribute__((visibility("default")))
#endif

#include <stdint.h>

extern "C" {

GENESYNTH_EXPORT void* GeneSynth_Create(double sampleRate);
GENESYNTH_EXPORT void GeneSynth_Destroy(void* instance);

/**
 * Returns the number of parameters exposed by the APVTS.
 */
GENESYNTH_EXPORT int GeneSynth_GetNumParameters(void* instance);

/**
 * Gets the parameter ID (string) for a given index.
 * Useful for mapping python dictionary to indices.
 */
GENESYNTH_EXPORT const char* GeneSynth_GetParameterId(void* instance, int index);

/**
 * Renders audio from the given parameters and extracts the Mel Spectrogram.
 * @param params Array of float values [0.0, 1.0]. Must have length == GeneSynth_GetNumParameters.
 * @param outSpectrogram Array of exactly 86 * 128 floats (11008).
 */
GENESYNTH_EXPORT void GeneSynth_RenderFeatures(void* instance, const float* params, float* outSpectrogram);

} // extern "C"
