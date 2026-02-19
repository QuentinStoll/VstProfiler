# to modify or make usefull
function(apply_compiler_options target)
    target_compile_features(${target} PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(${target} PRIVATE
            /W4
            /utf-8
            $<$<CONFIG:Release>:/O2 /GL>
        )
        target_link_options(${target} PRIVATE
            $<$<CONFIG:Release>:/LTCG>
        )
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Debug>:-g>
        )
    endif()

    if(APPLE)
        target_compile_options(${target} PRIVATE
            -Wno-unknown-pragmas
        )
    endif()
endfunction()

function(apply_plugin_definitions target)
    if(ENABLE_ASSERTIONS)
        target_compile_definitions(${target} PRIVATE ENABLE_ASSERTIONS=1)
    endif()

    if(ENABLE_LOGGING)
        target_compile_definitions(${target} PRIVATE ENABLE_LOGGING=1)
    endif()

    if(ENABLE_PERF_LOGGING)
        target_compile_definitions(${target} PRIVATE ENABLE_PERF_LOGGING=1)
    endif()

    if(ENABLE_DSP_TRACE)
        target_compile_definitions(${target} PRIVATE ENABLE_DSP_TRACE=1)
    endif()
endfunction()