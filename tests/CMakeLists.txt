add_library(unit_tests_lib OBJECT
    config_scanner_tests.cpp
    file_inspector_tests.cpp
    socket_tests.cpp
    server_tests.cpp
    client_tests.cpp
    stats_util_tests.cpp
)


target_include_directories(unit_tests_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(unit_tests_lib PUBLIC
    GTest::gtest 
    nlohmann_json::nlohmann_json
    config_scanner_lib
    file_inspector_lib
    socket_lib
    server_lib
    client_lib
    stats_util_lib
)

target_compile_options(unit_tests_lib PRIVATE --coverage -O0 -g)
target_link_options(unit_tests_lib PRIVATE --coverage)

add_subdirectory(integration_tests)

add_executable(tests
    main.cpp
)
target_link_libraries(tests PUBLIC
    unit_tests_lib
    integration_tests_lib
)

include(GoogleTest)