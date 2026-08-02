#pragma once

namespace genesynth {

class PitchTracker {
public:
    /**
     * Detects the fundamental frequency (pitch) of an audio signal using the McLeod Pitch Method (MPM).
     * 
     * @param audioData Pointer to the audio samples
     * @param numSamples Number of samples
     * @param sampleRate The sample rate of the audio data
     * @return The fundamental frequency in Hz, or 0.0f if no pitch is detected
     */
    static float detectPitchMPM(const float* audioData, int numSamples, double sampleRate);
};

} // namespace genesynth
