include(FetchContent)

set(FETCHCONTENT_QUIET OFF)



# JUCE ----------------------

# no option for use of local JUCE (to many breaking changes)
if(NOT TARGET juce::juce_core)
    message(STATUS "Fetching JUCE from repository")
    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()


# spdlog --------------------

option(USE_SYSTEM_SPDLOG "Use system-installed spdlog via find_package" OFF) # only for local builds
if (TARGET spdlog::spdlog)
    message(STATUS "spdlog: using existing target")
elseif (PROFILER_USE_SYSTEM_SPDLOG)
    message(STATUS "spdlog: using system package")
    find_package(spdlog CONFIG REQUIRED)
else()
    FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.16.0
    )

    set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(spdlog)
endif()
