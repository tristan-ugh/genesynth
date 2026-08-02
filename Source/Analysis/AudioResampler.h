#pragma once
#include <JuceHeader.h>

namespace genesynth {

class AudioResampler {
public:
    static constexpr float C5_FREQUENCY = 523.251f;

    /**
     * Resamples the given audio buffer so that its fundamental frequency is shifted to C5 (523.25 Hz).
     * This changes the length of the audio buffer (classic tape-style repitch).
     * 
     * @param originalBuffer The source audio buffer
     * @param originalFundFreq The detected fundamental frequency of the source
     * @return A new AudioBuffer containing the repitched audio
     */
    static juce::AudioBuffer<float> repitchToC5(const juce::AudioBuffer<float>& originalBuffer, float originalFundFreq);

    /**
     * Resamples the audio buffer from its original sample rate to a target sample rate.
     * This is useful to standardize the input for ML models (e.g. 22050 Hz).
     */
    static juce::AudioBuffer<float> resampleToTargetRate(const juce::AudioBuffer<float>& originalBuffer, double originalRate, double targetRate);
};

} // namespace genesynth
