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


# RTNeural ------------------

if(NOT TARGET RTNeural)
    message(STATUS "Fetching RTNeural")
    FetchContent_Declare(
        rtneural
        GIT_REPOSITORY https://github.com/jatinchowdhury18/RTNeural.git
        GIT_TAG main
    )

    set(RTNEURAL_BACKEND "STL" CACHE STRING "" FORCE)
    set(RTNEURAL_XSIMD OFF CACHE BOOL "" FORCE)
    set(RTNEURAL_USE_JSON ON CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(rtneural)
endif()


# tracy ---------------------

option(USE_SYSTEM_TRACY "Use system-installed Tracy via find_package" OFF) # local builds only
if (TARGET Tracy::TracyClient)
    message(STATUS "Tracy: using existing target")
elseif (USE_SYSTEM_TRACY)
    message(STATUS "Tracy: using system package")
    find_package(Tracy CONFIG REQUIRED)
else()
    include(FetchContent)
    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG 0350df1 # v0.13.4 do change at your own risk if you have a protocol missmatch with the tracy binaries
    )

    if(ENABLE_TRACY_CLIENT)
        set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    endif()
    set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
    set(TRACY_ONLY_LOCALHOST OFF CACHE BOOL "" FORCE)
    set(TRACY_NO_EXIT OFF CACHE BOOL "" FORCE)
    set(TRACY_NO_CONTEXT_SWITCH OFF CACHE BOOL "" FORCE)
    set(TRACY_NO_SAMPLING OFF CACHE BOOL "" FORCE)
    set(TRACY_NO_SYSTEM_TRACING OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(tracy)
endif()


# gtk workaround (linux) ----

if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.1)
    # pkg_check_modules(CURL REQUIRED libcurl)
    include_directories(${GTK3_INCLUDE_DIRS} ${WEBKIT2GTK_INCLUDE_DIRS})
endif()


