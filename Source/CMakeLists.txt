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

set_target_properties(${PLUGIN_NAME}  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/arc"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

juce_add_binary_data(ProfilerAssets
    SOURCES
        Assets/Svg/cross-icon.svg
        Assets/Svg/music-note.svg
        Assets/Svg/plus-icon.svg
)

# Collect source files
target_sources(${PLUGIN_NAME} PRIVATE
    Src/Components/CustomComboBox.cpp
    Src/Components/CustomImageButton.cpp
    Src/Components/CustomKnob.cpp
    Src/Components/CustomLinearSlider.cpp
    Src/Components/CustomTabBar.cpp
    Src/Components/CustomTabs.cpp
    Src/Components/CustomTextButton.cpp
    Src/Components/CustomTextEditor.cpp
    Src/Components/CustomToggleButton.cpp
    Src/Components/FormFields/FormComboField.cpp
    Src/Components/FormComponent.cpp
    Src/Components/FormFields/FormField.cpp
    Src/Components/FormFields/FormPathField.cpp
    Src/Components/FormFields/FormSliderField.cpp
    Src/Components/FormFields/FormTextField.cpp
    Src/Components/FormFields/FormToggleField.cpp
    Src/Components/FileStatusCard.cpp
    Src/Components/ModalOverlay.cpp
    Src/Components/NotificationBanner.cpp
    Src/Modules/AddProfilModule.cpp
    Src/Modules/AdvancedEqModule.cpp
    Src/Modules/BasicEqModule.cpp
    Src/Modules/CardGridModule.cpp
    Src/Modules/CreateProfilModule.cpp
    Src/Modules/DeleteProfileConfirmationModule.cpp
    Src/Modules/EditProfilModule.cpp
    Src/Modules/ExportProfilModule.cpp
    Src/Modules/FileAssetsModule.cpp
    Src/Modules/MasterSlidersModule.cpp
    Src/Modules/UtilityBarModule.cpp
    Src/Views/CloneView.cpp
    Src/Views/PlayView.cpp
    Src/Views/ProfilView.cpp
    Src/ProfileManager.cpp
    Src/PluginEditor.cpp
    Src/PluginProcessor.cpp
    Src/Logging.cpp
)

# Create plugin library
# add_library(${PLUGIN_NAME} STATIC ${SOURCES})
add_compile_definitions(JUCE_VST3_CAN_REPLACE_VST2=0)

# Link JUCE modules from juce_setup.cmake
if(NOT WIN32)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE} ProfilerAssets curl simdjson TracyClient)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE} curl simdjson RTNeural TracyClient)
else()
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE} ProfilerAssets simdjson TracyClient)
    target_link_libraries(${PLUGIN_NAME} PRIVATE ${JUCE_MODULES_AVAILABLE} simdjson RTNeural TracyClient)
endif()

# Include directories
target_include_directories(${PLUGIN_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../JuceLibraryCode
    ${rtneural_SOURCE_DIR}
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
