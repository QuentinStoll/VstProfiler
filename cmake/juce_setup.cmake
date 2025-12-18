if(NOT TARGET juce::juce_core)
    message(FATAL_ERROR "JUCE not found. Make sure dependencies.cmake is included first.")
endif()

# Legacy tool build only
option(BUILD_PROJUCER "Build JUCE Projucer (legacy only)" OFF)
if(EXISTS "${CMAKE_SOURCE_DIR}/Libs/JUCE" AND NOT TARGET Projucer AND BUILD_PROJUCER)
    message(STATUS "Building Projucer for legacy compatibility")

    if(EXISTS "${CMAKE_SOURCE_DIR}/juce/JUCE/extras/Projucer/CMakeLists.txt")
        add_subdirectory(juce/JUCE/extras/Projucer EXCLUDE_FROM_ALL)
        set_target_properties(Projucer PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            FOLDER "Compatibility"
        )
    endif()
endif()


# Juce Setup
# plugin format check
if(NOT JUCE_SUPPORTS_AAX AND PLUGIN_AAX)
    message(WARNING "AAX requested but not available in this JUCE installation")
    set(PLUGIN_AAX false)
endif()

if(NOT APPLE AND PLUGIN_AU)
    message(WARNING "AU format only available on macOS")
    set(PLUGIN_AU false)
endif()

# disable unecessary juce builds
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "")

juce_disable_default_flags()

set(JUCE_MODULES_AVAILABLE
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    juce_core
    juce_data_structures
    juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
)