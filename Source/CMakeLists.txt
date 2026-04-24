# Plugin compiling
set(PLUGIN_NAME Profiler)

juce_add_plugin(${PLUGIN_NAME}
    COMPANY_NAME "YourCompany"
    PRODUCT_NAME "Profiler"
    PLUGIN_NAME "Profiler"
    FORMATS AU VST3 Standalone   # Sera surchargé par tes presets plus bas
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/arc"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Collect source files
target_sources(${PLUGIN_NAME} PRIVATE
    Src/AmpLoaderSection.cpp
    Src/CloningScreen.cpp
    Src/EqNormalizeSection.cpp
    Src/MainSettingsSection.cpp
    Src/PluginEditor.cpp
    Src/PluginProcessor.cpp
    Src/RotarySlider.cpp
    Src/SelectionScreen.cpp
    Src/UsingScreen.cpp
    Src/ViewManager.cpp
    Src/AmpEngine.cpp
    Src/AmpProfiling.cpp
    Src/Logging.cpp
)

add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)

# Link libraries - Intégration de RTNeural et Eigen
if(NOT WIN32)
    target_link_libraries(${PLUGIN_NAME} PRIVATE 
        ${JUCE_MODULES_AVAILABLE} 
        juce::juce_dsp        # Requis pour l'audio IA
        curl 
        simdjson 
        RTNeural              # Ajout IA
        Eigen3::Eigen         # Ajout IA
    )
else()
    target_link_libraries(${PLUGIN_NAME} PRIVATE 
        ${JUCE_MODULES_AVAILABLE} 
        juce::juce_dsp        # Requis pour l'audio IA
        simdjson 
        RTNeural              # Ajout IA
        Eigen3::Eigen         # Ajout IA
    )
endif()

# Définitions pour optimiser l'IA
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    RTNEURAL_USE_EIGEN=1
    RTNEURAL_DEFAULT_ALIGNMENT=32
)

# Optimisation matérielle spécifique Windows
if(MSVC)
    target_compile_options(${PLUGIN_NAME} PRIVATE /arch:AVX2)
endif()

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../JuceLibraryCode
)

# Apply compiler options and preset-based defines
apply_compiler_options(${PLUGIN_NAME})

# Add plugin formats based on presets
if(APPLE)
    set(FORMATS AU VST3 Standalone)
elseif(UNIX)
    set(FORMATS VST3)
else()
    set(FORMATS VST3 Standalone)
endif()