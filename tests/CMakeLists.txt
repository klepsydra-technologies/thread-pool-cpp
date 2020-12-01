#tests

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../spdlog/include)

function(build_test test_name)
    add_executable(${test_name}_test ${ARGN}) 
    target_link_libraries(${test_name}_test pthread gtest gtest_main)
    add_test(${test_name} ./${test_name}_test)
endfunction()

build_test(fixed_function fixed_function.t.cpp)
build_test(thread_pool thread_pool.t.cpp)
build_test(non_blocking_thread_pool non_blocking_thread_pool.t.cpp)
build_test(thread_pool_options thread_pool_options.t.cpp)
