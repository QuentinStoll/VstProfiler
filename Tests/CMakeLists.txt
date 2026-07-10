set(PROFILER_TEST_SOURCES
    TestRunner.cpp
    AudioProcessorTests.cpp
    ProfileProcessorIntegrationTests.cpp
    ProfileManagerTests.cpp
)

add_executable(ProfilerTests
    ${PROFILER_TEST_SOURCES}
    ../Source/Src/Logging.cpp
    ../Source/Src/PluginProcessor.cpp
    ../Source/Src/ProfileManager.cpp
)

target_include_directories(ProfilerTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Support
    ${CMAKE_SOURCE_DIR}/Source/Include
)

target_compile_definitions(ProfilerTests PRIVATE
    JUCE_UNIT_TESTS=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    PROFILER_HEADLESS_TESTS=1
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_Name="Profiler"
    PROFILER_TEST_TEMP_ROOT="${CMAKE_CURRENT_BINARY_DIR}/tmp"
)

target_link_libraries(ProfilerTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    RTNeural
    simdjson
)

apply_compiler_options(ProfilerTests)

function(add_profiler_profile_manager_test test_name case_name)
    add_test(NAME ProfilerTests.ProfileManager.${test_name}
        COMMAND ProfilerTests --case "${case_name}")
endfunction()

add_profiler_profile_manager_test(CreateProfile "create profile")
add_profiler_profile_manager_test(UpdateProfile "update profile")
add_profiler_profile_manager_test(DeleteProfile "delete profile")
add_profiler_profile_manager_test(ExportProfile "export profile")
add_profiler_profile_manager_test(ImportProfile "import profile")
add_profiler_profile_manager_test(RejectOutOfRangeNumericValue "reject out-of-range numeric value")
add_profiler_profile_manager_test(RejectNonStringPathValue "reject non-string path value")
add_profiler_profile_manager_test(RejectInvalidProfileJson "reject invalid profile JSON")
add_profiler_profile_manager_test(RejectUnsupportedProfileFormat "reject unsupported profile format")
add_profiler_profile_manager_test(ApplyParameterMapping "apply profile values to APVTS parameters")
add_profiler_profile_manager_test(SaveCurrentProfileSetting "save current profile setting")
add_profiler_profile_manager_test(ClearCurrentProfileSetting "clear current profile setting")
add_profiler_profile_manager_test(CaptureCurrentValues "capture current APVTS values with profile file paths")

function(add_profiler_audio_processor_test test_name case_name)
    add_test(NAME ProfilerTests.AudioProcessor.${test_name}
        COMMAND ProfilerTests --case "${case_name}")
endfunction()

add_profiler_audio_processor_test(Metadata "audio processor metadata")
add_profiler_audio_processor_test(BusLayouts "audio processor bus layouts")
add_profiler_audio_processor_test(ParameterDefaults "audio processor parameter defaults")
add_profiler_audio_processor_test(StateRoundTrip "audio processor state round trip")
add_profiler_audio_processor_test(ProcessBlockSmoke "audio processor process block smoke")
add_profiler_audio_processor_test(MuteClearsBuffer "audio processor mute clears buffer")
add_profiler_audio_processor_test(MasterZeroSilencesBuffer "audio processor master zero silences buffer")
add_profiler_audio_processor_test(GainChangesOutputLevel "audio processor gain changes output level")
add_profiler_audio_processor_test(SilentInputStaysSilent "audio processor silent input stays silent")
add_profiler_audio_processor_test(PrepareBlockSizes "audio processor handles prepare block sizes")
add_profiler_audio_processor_test(ReleaseAndReprepare "audio processor release and reprepare")
add_profiler_audio_processor_test(RejectMissingAssetFiles "audio processor rejects missing asset files")

function(add_profiler_profile_processor_integration_test test_name case_name)
    add_test(NAME ProfilerTests.ProfileProcessorIntegration.${test_name}
        COMMAND ProfilerTests --case "${case_name}")
endfunction()

add_profiler_profile_processor_integration_test(ApplyMapsProfileValues "profile processor apply maps profile values")
add_profiler_profile_processor_integration_test(TracksProfileAssetPaths "profile processor tracks profile asset paths")
add_profiler_profile_processor_integration_test(AppliedProfileAffectsAudio "profile processor applied profile affects audio")
add_profiler_profile_processor_integration_test(SyncsCurrentProfileSelection "profile processor syncs current profile selection")
