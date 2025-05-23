cmake_minimum_required(VERSION 2.8)

project(thread-pool-cpp CXX C)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
if(COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# gtest
enable_testing()
add_subdirectory(googletest)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Coverage
if (CMAKE_BUILD_TYPE MATCHES Debug AND CMAKE_COMPILER_IS_GNUCXX)
    include(cmake/CodeCoverage.cmake)
    set(CODE_COVERAGE_VERBOSE ON)
    append_coverage_compiler_flags()
    set(COVERAGE_EXCLUDES "benchmark" "*build*" "googletest" "spdlog" "tests")
    setup_target_for_coverage_gcovr_xml(NAME coverage-xml EXECUTABLE make all test)
    setup_target_for_coverage_gcovr_html(NAME coverage-html EXECUTABLE make all test)
endif ()

# Tests
add_subdirectory(tests)

# Benchmark
add_subdirectory(benchmark)

# Install
file(GLOB_RECURSE INSTALL_FILES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/include/*")
set_source_files_properties(${INSTALL_FILES_LIST} PROPERTIES HEADER_FILE_ONLY 1)
add_library(HEADER_ONLY_TARGET STATIC ${INSTALL_FILES_LIST})
set_target_properties(HEADER_ONLY_TARGET PROPERTIES LINKER_LANGUAGE CXX)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "include")
