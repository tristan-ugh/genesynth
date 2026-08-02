cmake_minimum_required(VERSION 3.24)
project(genesynth_prototype VERSION 0.1.0)

# ==============================================================================
# Options
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Fetch JUCE
# ==============================================================================
include(FetchContent)
message(STATUS "Fetching JUCE...")
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.0
)
FetchContent_MakeAvailable(juce)

# ==============================================================================
# Fetch ONNX Runtime (Pre-compiled)
# ==============================================================================
if(WIN32)
    set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-win-x64-1.17.1.zip")
elseif(UNIX AND NOT APPLE)
    set(ONNXRUNTIME_URL "https://github.com/microsoft/onnxruntime/releases/download/v1.17.1/onnxruntime-linux-x64-1.17.1.tgz")
endif()

message(STATUS "Fetching ONNX Runtime...")
FetchContent_Declare(
    onnxruntime
    URL ${ONNXRUNTIME_URL}
)
FetchContent_MakeAvailable(onnxruntime)

add_library(onnxruntime_lib SHARED IMPORTED)
if(WIN32)
    set_target_properties(onnxruntime_lib PROPERTIES
        IMPORTED_LOCATION "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
        IMPORTED_IMPLIB "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${onnxruntime_SOURCE_DIR}/include"
    )
elseif(UNIX AND NOT APPLE)
    set_target_properties(onnxruntime_lib PROPERTIES
        IMPORTED_LOCATION "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so"
        INTERFACE_INCLUDE_DIRECTORIES "${onnxruntime_SOURCE_DIR}/include"
    )
endif()

# ==============================================================================
# Setup Plugin Target
# ==============================================================================
set(JUCE_WEB_BROWSER 0 CACHE BOOL "")
set(JUCE_USE_CURL 0 CACHE BOOL "")

juce_add_plugin(genesynth_prototype
    # ID et formats
    PLUGIN_MANUFACTURER_CODE "Tgsy"
    PLUGIN_CODE "Gsnt"
    FORMATS VST3 Standalone
    PRODUCT_NAME "GeneSynth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
)

# Définition des sources
target_sources(genesynth_prototype PRIVATE
    Source/PluginEditor.cpp
    Source/PluginProcessor.cpp
    Source/Analysis/PitchTracker.cpp
    Source/Analysis/AudioResampler.cpp
    Source/Analysis/FeatureExtractor.cpp
    Source/Analysis/InferenceEngine.cpp
    Source/Style/GeneSynthLookAndFeel.cpp
    Source/Parameters/ModulableParameter.cpp
    Source/Parameters/SynthParameter.cpp
    Source/Parameters/StaticParameter.cpp
    Source/Parameters/ParameterFactory.cpp
    Source/UI/SynthComponents.cpp
    Source/UI/Visualizers.cpp
    Source/UI/Views/SynthView.cpp
    Source/UI/Views/MorphView.cpp
    Source/Framework/iBlock.cpp
    Source/Framework/aBlock/aBlock.cpp
    Source/Framework/aBlock/aContainer/aContainer.cpp
    Source/Framework/aBlock/aContainer/ParallelContainer.cpp
    Source/Framework/aBlock/aContainer/SeriesContainer.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/GlobalModMatrix.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/SynthEngine.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/PolyphonyManager.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/SynthVoice.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/GlobalAmp.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/FilterSection.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/SoftClipper/SoftClipper.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/FilterSection/LadderFilter/LadderFilter.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/ExcitationSection.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aGenerator.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/Noise/NoiseGenerator.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/aOsc.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/OscB/OscB.cpp
    Source/Framework/aBlock/aContainer/SynthEngine/PolyphonyManager/SynthVoice/Excitation/aGenerator/aOsc/OscA/OscA.cpp
    Source/Framework/aBlock/ReverbModule/ReverbModule.cpp
)

# Inclusion des en-têtes
target_include_directories(genesynth_prototype PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    src
)

# Lien avec JUCE et ONNX Runtime
target_link_libraries(genesynth_prototype PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_plugin_client
    juce::juce_dsp
    onnxruntime_lib
)

if(WIN32)
    target_link_options(genesynth_prototype_VST3 PRIVATE "/DELAYLOAD:onnxruntime.dll")
    target_link_libraries(genesynth_prototype_VST3 PRIVATE delayimp.lib)
    
    target_link_options(genesynth_prototype_Standalone PRIVATE "/DELAYLOAD:onnxruntime.dll")
    target_link_libraries(genesynth_prototype_Standalone PRIVATE delayimp.lib)
endif()

# Copy ONNX Runtime shared library to output directories BEFORE juce_vst3_helper runs
if(WIN32)
    add_custom_command(TARGET genesynth_prototype_Standalone PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
        "$<TARGET_FILE_DIR:genesynth_prototype_Standalone>"
    )
    add_custom_command(TARGET genesynth_prototype_VST3 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:genesynth_prototype_VST3>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
        "$<TARGET_FILE_DIR:genesynth_prototype_VST3>"
        # Copy to the build directory where juce_vst3_helper.exe lives
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
        "${CMAKE_BINARY_DIR}/Release"
    )
elseif(UNIX AND NOT APPLE)
    add_custom_command(TARGET genesynth_prototype_Standalone PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so"
        "$<TARGET_FILE_DIR:genesynth_prototype_Standalone>"
    )
    add_custom_command(TARGET genesynth_prototype_VST3 PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:genesynth_prototype_VST3>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so"
        "$<TARGET_FILE_DIR:genesynth_prototype_VST3>"
    )
endif()

# Désactiver le WebBrowserComponent pour éviter la dépendance webkit2gtk-4.0 obsolète sous Linux
target_compile_definitions(genesynth_prototype PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(genesynth_prototype)

# ==============================================================================
# Python C API Library
# ==============================================================================
add_library(genesynth_python_api SHARED
    Source/API/GeneSynthAPI.cpp
)

target_include_directories(genesynth_python_api PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)

target_link_libraries(genesynth_python_api PRIVATE
    genesynth_prototype
)

if(WIN32)
    target_link_options(genesynth_python_api PRIVATE "/DELAYLOAD:onnxruntime.dll")
    target_link_libraries(genesynth_python_api PRIVATE delayimp.lib)
    
    add_custom_command(TARGET genesynth_python_api PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/onnxruntime.dll"
        "$<TARGET_FILE_DIR:genesynth_python_api>"
    )
elseif(UNIX AND NOT APPLE)
    add_custom_command(TARGET genesynth_python_api PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${onnxruntime_SOURCE_DIR}/lib/libonnxruntime.so"
        "$<TARGET_FILE_DIR:genesynth_python_api>"
    )
endif()
