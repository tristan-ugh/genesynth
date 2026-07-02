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

# No ONNX Runtime for now

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
    Source/Style/GeneSynthLookAndFeel.cpp
    Source/Parameters/ModulableParameter.cpp
    Source/Parameters/SynthParameter.cpp
    Source/Parameters/StaticParameter.cpp
    Source/Parameters/ParameterFactory.cpp
    Source/UI/SynthComponents.cpp
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
)

# Désactiver le WebBrowserComponent pour éviter la dépendance webkit2gtk-4.0 obsolète sous Linux
target_compile_definitions(genesynth_prototype PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(genesynth_prototype)
