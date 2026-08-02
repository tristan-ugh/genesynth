#include "InferenceEngine.h"

#if defined(_WIN32)
#include <windows.h>
#endif

namespace genesynth {

#if defined(_WIN32)
static void dummyFuncForModuleHandle() {}
#endif

InferenceEngine::InferenceEngine() : juce::Thread("InferenceEngine") {
#if defined(_WIN32)
    HMODULE hModule = NULL;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)&dummyFuncForModuleHandle,
                       &hModule);
    if (hModule != NULL) {
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(hModule, path, MAX_PATH);
        juce::File dllFile = juce::File(juce::String(path)).getParentDirectory().getChildFile("onnxruntime.dll");
        if (dllFile.existsAsFile()) {
            LoadLibraryW(dllFile.getFullPathName().toWideCharPointer());
        }
    }
#endif
    try {
        env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "GeneSynthInference");
        memoryInfo = std::make_unique<Ort::MemoryInfo>(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU));
    } catch (const Ort::Exception& e) {
        juce::Logger::writeToLog("ONNX Runtime initialization failed: " + juce::String(e.what()));
    }
    startThread();
}

InferenceEngine::~InferenceEngine() {
    stopThread(2000);
}

bool InferenceEngine::loadModel(const juce::String& modelPath) {
    if (!env) return false;
    
    juce::ScopedLock sl(inputLock);
    modelLoaded = false;
    session.reset();
    
    allocatedInputStrings.clear();
    allocatedOutputStrings.clear();
    inputNames.clear();
    outputNames.clear();
    
    try {
        Ort::SessionOptions sessionOptions;
        sessionOptions.SetIntraOpNumThreads(1);
        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        
#if defined(_WIN32)
        std::wstring wpath = modelPath.toWideCharPointer();
        session = std::make_unique<Ort::Session>(*env, wpath.c_str(), sessionOptions);
#else
        session = std::make_unique<Ort::Session>(*env, modelPath.toRawUTF8(), sessionOptions);
#endif
        
        Ort::AllocatorWithDefaultOptions allocator;
        
        size_t numInputNodes = session->GetInputCount();
        for (size_t i = 0; i < numInputNodes; i++) {
            Ort::AllocatedStringPtr name = session->GetInputNameAllocated(i, allocator);
            allocatedInputStrings.push_back(std::string(name.get()));
            inputNames.push_back(allocatedInputStrings.back().c_str());
            
            Ort::TypeInfo typeInfo = session->GetInputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            inputDims = tensorInfo.GetShape();
        }
        
        size_t numOutputNodes = session->GetOutputCount();
        for (size_t i = 0; i < numOutputNodes; i++) {
            Ort::AllocatedStringPtr name = session->GetOutputNameAllocated(i, allocator);
            allocatedOutputStrings.push_back(std::string(name.get()));
            outputNames.push_back(allocatedOutputStrings.back().c_str());
            
            Ort::TypeInfo typeInfo = session->GetOutputTypeInfo(i);
            auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();
            outputDims = tensorInfo.GetShape();
        }
        
        modelLoaded = true;
        juce::Logger::writeToLog("ONNX Model loaded successfully: " + modelPath);
        return true;
    } catch (const Ort::Exception& e) {
        juce::Logger::writeToLog("ONNX Model loading failed (Ort): " + juce::String(e.what()));
        return false;
    } catch (const std::exception& e) {
        juce::Logger::writeToLog("ONNX Model loading failed (std): " + juce::String(e.what()));
        return false;
    } catch (...) {
        juce::Logger::writeToLog("ONNX Model loading failed (unknown exception)");
        return false;
    }
}

void InferenceEngine::requestInference(const std::vector<float>& inputFeatures) {
    if (!modelLoaded) return;
    
    juce::ScopedLock sl(inputLock);
    currentInput = inputFeatures;
    inferenceRequested = true;
    notify(); // Wake up thread
}

void InferenceEngine::run() {
    while (!threadShouldExit()) {
        wait(-1); // Wait for notify()
        
        if (threadShouldExit()) break;
        
        if (inferenceRequested && modelLoaded) {
            std::vector<float> inputData;
            {
                juce::ScopedLock sl(inputLock);
                inputData = currentInput;
                inferenceRequested = false;
            }
            
            try {
                // Ensure shape matches
                int64_t totalInputElements = 1;
                for (auto dim : inputDims) totalInputElements *= (dim < 0 ? 1 : dim); 
                
                if (inputData.size() < (size_t)totalInputElements) {
                    inputData.resize(totalInputElements, 0.0f);
                }
                
                // Determine actual shape for this run
                std::vector<int64_t> currentInputDims = inputDims;
                int64_t knownElements = 1;
                int dynamicDimIndex = -1;
                
                for (size_t i = 0; i < currentInputDims.size(); ++i) {
                    if (currentInputDims[i] > 0) {
                        knownElements *= currentInputDims[i];
                    } else if (i == 0) {
                        currentInputDims[i] = 1; // Batch size = 1
                    } else {
                        dynamicDimIndex = (int)i;
                    }
                }
                
                if (dynamicDimIndex != -1) {
                    currentInputDims[dynamicDimIndex] = inputData.size() / knownElements;
                }
                
                Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
                    *memoryInfo, inputData.data(), inputData.size(), currentInputDims.data(), currentInputDims.size());
                
                auto outputTensors = session->Run(Ort::RunOptions{nullptr}, 
                                                  inputNames.data(), &inputTensor, 1, 
                                                  outputNames.data(), outputNames.size());
                
                if (!outputTensors.empty()) {
                    float* floatArr = outputTensors.front().GetTensorMutableData<float>();
                    auto typeInfo = outputTensors.front().GetTensorTypeAndShapeInfo();
                    size_t count = typeInfo.GetElementCount();
                    
                    std::vector<float> result(floatArr, floatArr + count);
                    
                    if (onInferenceComplete) {
                        // Dispatch to message thread
                        juce::MessageManager::callAsync([this, result]() {
                            onInferenceComplete(result);
                        });
                    }
                }
            } catch (const Ort::Exception& e) {
                juce::Logger::writeToLog("Inference failed: " + juce::String(e.what()));
            }
        }
    }
}

} // namespace genesynth
