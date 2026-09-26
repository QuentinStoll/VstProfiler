set(SPECTRA_DSP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../SpectraDspDLL" CACHE PATH
    "Path to the SpectraDspDLL source tree. Leave empty to build the plugin without the private engine.")

if(SPECTRA_DSP_SOURCE_DIR AND EXISTS "${SPECTRA_DSP_SOURCE_DIR}/CMakeLists.txt")
    add_subdirectory("${SPECTRA_DSP_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_spectra_dsp" EXCLUDE_FROM_ALL)
endif()

function(profiler_attach_spectra_dsp target)
    if(NOT TARGET SpectraDsp OR NOT TARGET ${target})
        return()
    endif()

    add_dependencies(${target} SpectraDsp)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:SpectraDsp>"
            "$<TARGET_FILE_DIR:${target}>"
        VERBATIM)
endfunction()
