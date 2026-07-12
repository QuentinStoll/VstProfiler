# Load presets from JSON

set(PRESETS_CONFIG_FILE "${CMAKE_CURRENT_LIST_DIR}/presets_config.json")
if(NOT EXISTS ${PRESETS_CONFIG_FILE})
    message(WARNING "${PRESETS_CONFIG_FILE} Presets config not found, using defaults")
    return()
endif()

file(READ ${PRESETS_CONFIG_FILE} PRESETS_JSON)
string(JSON PRESET_COUNT LENGTH "${PRESETS_JSON}" presets)
math(EXPR PRESET_MAX "${PRESET_COUNT} - 1")

if(NOT DEFINED PRESET_NAME)
    set(PRESET_NAME "default")
endif()

foreach(idx RANGE ${PRESET_MAX})
    string(JSON name GET "${PRESETS_JSON}" presets ${idx} name)

    if(name STREQUAL PRESET_NAME)
        string(JSON PLUGIN_VST3 ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} formats vst3)
        string(JSON PLUGIN_AU ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} formats au)
        string(JSON PLUGIN_AAX ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} formats aax)
        string(JSON PLUGIN_STANDALONE ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} formats standalone)

        string(JSON BUILD_TYPE ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} buildType)
        if(NOT BUILD_TYPE STREQUAL "NOTFOUND" AND NOT CMAKE_BUILD_TYPE)
            set(CMAKE_BUILD_TYPE ${BUILD_TYPE} CACHE STRING "" FORCE)
        endif()

        string(JSON ENABLE_ASSERTIONS ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} features assertions)
        string(JSON ENABLE_LOGGING ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} features logging)
        string(JSON ENABLE_TRACY_CLIENT ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} features tracyEnabled)
        string(JSON ENABLE_DSP_TRACE ERROR_VARIABLE err GET "${PRESETS_JSON}" presets ${idx} features dspTrace)

        message(STATUS "Loaded preset: ${PRESET_NAME}")
        return()
    endif()
endforeach()

message(WARNING "Preset '${PRESET_NAME}' not found, using defaults")

set(PLUGIN_VST3 true)
set(PLUGIN_AU false)
set(PLUGIN_AAX false)
set(PLUGIN_STANDALONE true)
set(ENABLE_ASSERTIONS false)
set(ENABLE_LOGGING true)
set(ENABLE_TRACY_CLIENT false)
set(ENABLE_DSP_TRACE false)