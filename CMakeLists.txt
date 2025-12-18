cmake_minimum_required(VERSION 3.22)

project(Profiler VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(Libs/JUCE)

juce_add_plugin(Profiler
    COMPANY_NAME "Epitech"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    PLUGIN_CODE "Prof"
    PLUGIN_MANUFACTURER_CODE "abcd"
    FORMATS VST3
    PRODUCT_NAME "Profiler"
    JUCE_VST3_CAN_REPLACE_VST2 TRUE)
	
juce_generate_juce_header(Profiler)

target_sources(Profiler PRIVATE
    Source/AmpLoaderSection.cpp
    Source/AmpLoaderSection.h
    Source/CloningScreen.cpp
    Source/CloningScreen.h
    Source/EqNormalizeSection.cpp
    Source/EqNormalizeSection.h
    Source/MainSettingsSection.cpp
    Source/MainSettingsSection.h
    Source/RotarySlider.cpp
    Source/RotarySlider.h
    Source/RotarySliderParameter.h
    Source/ScreenID.h
    Source/SelectionScreen.cpp
    Source/SelectionScreen.h
    Source/SweepGenerator.h
    Source/UsingScreen.cpp
    Source/UsingScreen.h
    Source/ViewManager.cpp
    Source/ViewManager.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h)

target_link_libraries(Profiler PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_graphics
    juce::juce_events
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_audio_formats
)

target_compile_features(Profiler PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(Profiler PRIVATE /W4)
endif()

target_compile_definitions(Profiler PRIVATE 
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)