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


# simdjson ------------------

option(USE_SYSTEM_SIMDJSON "Use system-installed simdjson via find_package" OFF) # local builds only
if (TARGET simdjson::simdjson)
    message(STATUS "simdjson: using existing target")
elseif (USE_SYSTEM_SIMDJSON)
    message(STATUS "simdjson: using system package")
    find_package(simdjson CONFIG REQUIRED)
else()
    FetchContent_Declare(
        simdjson
        GIT_REPOSITORY https://github.com/simdjson/simdjson.git
        GIT_TAG v4.2.4
    )

    set(SIMDJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SIMDJSON_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(SIMDJSON_BUILD_STATIC ON CACHE BOOL "" FORCE)
    set(SIMDJSON_ENABLE_THREADS ON CACHE BOOL "" FORCE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    FetchContent_MakeAvailable(simdjson)
endif()


# gtk workaround (linux) ----

if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0>=2.40)
    # pkg_check_modules(CURL REQUIRED libcurl)
    include_directories(${GTK3_INCLUDE_DIRS} ${WEBKIT2GTK_INCLUDE_DIRS})
endif()


