#pragma once
#include <JuceHeader.h>
#include <vector>

namespace genesynth {

struct FeatureData {
    std::vector<float> logMelSpectrogram; // Flattened 2D array [numFrames * numMels]
    std::vector<float> mfccs;             // Flattened 2D array [numFrames * numMfccs]
    int numFrames = 0;
    int numMels = 0;
    int numMfccs = 0;
};

class FeatureExtractor {
public:
    FeatureExtractor();
    ~FeatureExtractor() = default;

    /**
     * Extracts Log-Mel Spectrogram and MFCCs from a monophonic audio buffer.
     * The input audio should ideally be pre-processed (resampled to 22050 Hz and repitched).
     * 
     * @param audioBuffer The input mono audio buffer. If stereo, only channel 0 is used.
     * @param sampleRate The sample rate of the input buffer (e.g., 22050.0)
     * @return FeatureData containing the extracted tensors
     */
    FeatureData extractFeatures(const juce::AudioBuffer<float>& audioBuffer, double sampleRate);

private:
    static constexpr int FFT_ORDER = 11; // 2048
    static constexpr int FFT_SIZE = 1 << FFT_ORDER;
    static constexpr int HOP_LENGTH = 256;
    static constexpr int NUM_MELS = 128;
    static constexpr int NUM_MFCCS = 13;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    
    // Mel filterbank weights: [NUM_MELS][FFT_SIZE / 2 + 1]
    std::vector<std::vector<float>> melFilterbank;
    double cachedSampleRate = 0.0;
    
    // DCT matrix for MFCCs: [NUM_MFCCS][NUM_MELS]
    std::vector<std::vector<float>> dctMatrix;

    void createMelFilterbank(double sampleRate);
    void createDCTMatrix();
    
    inline float hzToMel(float hz) const { return 2595.0f * std::log10(1.0f + hz / 700.0f); }
    inline float melToHz(float mel) const { return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f); }
};

} // namespace genesynth
