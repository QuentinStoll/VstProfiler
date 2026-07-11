if(NOT DEFINED PROFILER_COVERAGE_XML)
    message(WARNING "Coverage XML path was not provided")
    return()
endif()

if(NOT EXISTS "${PROFILER_COVERAGE_XML}")
    message(WARNING "Coverage XML was not found: ${PROFILER_COVERAGE_XML}")
    return()
endif()

file(READ "${PROFILER_COVERAGE_XML}" _profiler_coverage_xml)

function(print_coverage_percent label covered valid)
    if(NOT valid GREATER 0)
        return()
    endif()

    math(EXPR _percent_x100 "(${covered} * 10000) / ${valid}")
    math(EXPR _percent_whole "${_percent_x100} / 100")
    math(EXPR _percent_fraction "${_percent_x100} % 100")

    if(_percent_fraction LESS 10)
        set(_percent_fraction "0${_percent_fraction}")
    endif()

    message(STATUS "Coverage ${label}: ${_percent_whole}.${_percent_fraction}% (${covered}/${valid})")
endfunction()

string(REGEX MATCH "lines-covered=\"([0-9]+)\"" _lines_covered_match "${_profiler_coverage_xml}")
set(_lines_covered "${CMAKE_MATCH_1}")

string(REGEX MATCH "lines-valid=\"([0-9]+)\"" _lines_valid_match "${_profiler_coverage_xml}")
set(_lines_valid "${CMAKE_MATCH_1}")

string(REGEX MATCH "branches-covered=\"([0-9]+)\"" _branches_covered_match "${_profiler_coverage_xml}")
set(_branches_covered "${CMAKE_MATCH_1}")

string(REGEX MATCH "branches-valid=\"([0-9]+)\"" _branches_valid_match "${_profiler_coverage_xml}")
set(_branches_valid "${CMAKE_MATCH_1}")

print_coverage_percent("lines" "${_lines_covered}" "${_lines_valid}")
print_coverage_percent("branches" "${_branches_covered}" "${_branches_valid}")
