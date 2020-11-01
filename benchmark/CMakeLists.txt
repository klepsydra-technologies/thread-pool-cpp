#benchmark

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../spdlog/include)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark pthread)

