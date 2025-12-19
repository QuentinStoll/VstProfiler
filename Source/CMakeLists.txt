# Plugin compiling

set(PLUGIN_NAME Profiler)

juce_add_plugin(${PLUGIN_NAME}
    COMPANY_NAME "YourCompany"
    PRODUCT_NAME "Profiler"
    PLUGIN_NAME "Profiler"

    FORMATS AU VST3 Standalone   # pick what you need
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

# Collect source files
target_sources(${PLUGIN_NAME} PRIVATE
    AmpLoaderSection.cpp
    CloningScreen.cpp
    EqNormalizeSection.cpp
    MainSettingsSection.cpp
    PluginEditor.cpp
    PluginProcessor.cpp
    RotarySlider.cpp
    SelectionScreen.cpp
    UsingScreen.cpp
    ViewManager.cpp
)

# Create plugin library
# add_library(${PLUGIN_NAME} STATIC ${SOURCES})

# Link JUCE modules from juce_setup.cmake
target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE})

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
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