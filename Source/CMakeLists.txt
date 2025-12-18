# Plugin compiling

set(PLUGIN_NAME Profiler)

# Collect source files
set(SOURCES
    AmpLoaderSection.cpp
    #AmpLoaderSection.h
    CloningScreen.cpp
    #CloningScreen.h
    EqNormalizeSection.cpp
    #EqNormalizeSection.h
    MainSettingsSection.cpp
    #MainSettingsSection.h
    PluginEditor.cpp
    #PluginEditor.h
    PluginProcessor.cpp
    #PluginProcessor.h
    RotarySlider.cpp
    #RotarySlider.h
    #RotarySliderParameter.h
    #ScreenID.h
    SelectionScreen.cpp
    #SelectionScreen.h
    #SweepGenerator.h
    UsingScreen.cpp
    #UsingScreen.h
    ViewManager.cpp
    #ViewManager.h
)

# Create plugin library
add_library(${PLUGIN_NAME} STATIC ${SOURCES})

# Link JUCE modules from juce_setup.cmake
target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE})

# Apply compiler options and preset-based defines
apply_compiler_options(${PLUGIN_NAME})

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../JuceLibraryCode
)

# Add plugin formats based on presets
if(PLUGIN_VST3)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE JUCE_PLUGINHOST_VST3)
endif()
if(PLUGIN_AU)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE JUCE_PLUGINHOST_AU)
endif()
if(PLUGIN_AAX)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE JUCE_PLUGINHOST_AAX)
endif()
if(PLUGIN_STANDALONE)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE JUCE_PLUGINHOST_STANDALONE)
endif()
