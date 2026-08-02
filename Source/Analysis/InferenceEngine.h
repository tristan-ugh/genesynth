#pragma once
#include <JuceHeader.h>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <atomic>
#include <memory>
#include <functional>

namespace genesynth {

class InferenceEngine : public juce::Thread {
public:
    InferenceEngine();
    ~InferenceEngine() override;

    // Load a model from file
    bool loadModel(const juce::String& modelPath);
    
    // Request an asynchronous inference
    void requestInference(const std::vector<float>& inputFeatures);
    
    // Thread payload
    void run() override;

    // Callback when inference finishes (called on the Message Thread)
    std::function<void(const std::vector<float>&)> onInferenceComplete;

private:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::Session> session;
    std::unique_ptr<Ort::MemoryInfo> memoryInfo;
    
    std::atomic<bool> modelLoaded { false };
    std::atomic<bool> inferenceRequested { false };
    
    std::vector<float> currentInput;
    juce::CriticalSection inputLock;
    
    // Model metadata
    std::vector<const char*> inputNames;
    std::vector<std::string> allocatedInputStrings;
    std::vector<const char*> outputNames;
    std::vector<std::string> allocatedOutputStrings;
    
    std::vector<int64_t> inputDims;
    std::vector<int64_t> outputDims;
};

} // namespace genesynth
