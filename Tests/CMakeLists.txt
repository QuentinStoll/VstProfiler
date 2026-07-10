set(PROFILER_TEST_SOURCES
    TestRunner.cpp
    ProfileManagerTests.cpp
)

add_executable(ProfilerTests
    ${PROFILER_TEST_SOURCES}
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
    PROFILER_TEST_TEMP_ROOT="${CMAKE_CURRENT_BINARY_DIR}/tmp"
)

target_link_libraries(ProfilerTests PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
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
