cmake_minimum_required(VERSION 3.16)
project(console_malware_analyzer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(external/googletest)
add_subdirectory(external/json)
add_subdirectory(external/csv-parser)
add_subdirectory(external/inih)

add_subdirectory(src)

option(BUILD_TESTS "Build tests" ON)
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
