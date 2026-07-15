set(_profiler_lines_covered 0)
set(_profiler_lines_valid 0)
set(_profiler_branches_covered 0)
set(_profiler_branches_valid 0)

if(DEFINED PROFILER_COVERAGE_XML)
    if(NOT EXISTS "${PROFILER_COVERAGE_XML}")
        message(FATAL_ERROR "Coverage XML was not found: ${PROFILER_COVERAGE_XML}")
    endif()

    file(READ "${PROFILER_COVERAGE_XML}" _profiler_coverage_xml)

    string(REGEX MATCH "lines-covered=\"([0-9]+)\"" _lines_covered_match "${_profiler_coverage_xml}")
    set(_profiler_lines_covered "${CMAKE_MATCH_1}")

    string(REGEX MATCH "lines-valid=\"([0-9]+)\"" _lines_valid_match "${_profiler_coverage_xml}")
    set(_profiler_lines_valid "${CMAKE_MATCH_1}")

    string(REGEX MATCH "branches-covered=\"([0-9]+)\"" _branches_covered_match "${_profiler_coverage_xml}")
    set(_profiler_branches_covered "${CMAKE_MATCH_1}")

    string(REGEX MATCH "branches-valid=\"([0-9]+)\"" _branches_valid_match "${_profiler_coverage_xml}")
    set(_profiler_branches_valid "${CMAKE_MATCH_1}")
elseif(DEFINED PROFILER_COVERAGE_LCOV)
    if(NOT EXISTS "${PROFILER_COVERAGE_LCOV}")
        message(FATAL_ERROR "LCOV report was not found: ${PROFILER_COVERAGE_LCOV}")
    endif()

    file(STRINGS "${PROFILER_COVERAGE_LCOV}" _profiler_lcov_totals REGEX "^(LF|LH|BRF|BRH):[0-9]+$")
    foreach(_coverage_total IN LISTS _profiler_lcov_totals)
        string(REGEX MATCH "^([A-Z]+):([0-9]+)$" _coverage_match "${_coverage_total}")
        set(_coverage_kind "${CMAKE_MATCH_1}")
        set(_coverage_value "${CMAKE_MATCH_2}")

        if(_coverage_kind STREQUAL "LF")
            math(EXPR _profiler_lines_valid "${_profiler_lines_valid} + ${_coverage_value}")
        elseif(_coverage_kind STREQUAL "LH")
            math(EXPR _profiler_lines_covered "${_profiler_lines_covered} + ${_coverage_value}")
        elseif(_coverage_kind STREQUAL "BRF")
            math(EXPR _profiler_branches_valid "${_profiler_branches_valid} + ${_coverage_value}")
        elseif(_coverage_kind STREQUAL "BRH")
            math(EXPR _profiler_branches_covered "${_profiler_branches_covered} + ${_coverage_value}")
        endif()
    endforeach()
else()
    message(FATAL_ERROR "A Cobertura XML or LCOV coverage report must be provided")
endif()

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

if(NOT _profiler_lines_valid GREATER 0)
    message(FATAL_ERROR "Coverage report does not contain any source lines")
endif()

print_coverage_percent("lines" "${_profiler_lines_covered}" "${_profiler_lines_valid}")
print_coverage_percent("branches" "${_profiler_branches_covered}" "${_profiler_branches_valid}")

if(NOT DEFINED PROFILER_COVERAGE_MINIMUM)
    set(PROFILER_COVERAGE_MINIMUM 60)
endif()

math(EXPR _profiler_covered_scaled "${_profiler_lines_covered} * 100")
math(EXPR _profiler_required_scaled "${_profiler_lines_valid} * ${PROFILER_COVERAGE_MINIMUM}")

if(_profiler_covered_scaled LESS _profiler_required_scaled)
    message(FATAL_ERROR "Line coverage is below the required ${PROFILER_COVERAGE_MINIMUM}%")
endif()

message(STATUS "Coverage threshold passed: at least ${PROFILER_COVERAGE_MINIMUM}% lines")
