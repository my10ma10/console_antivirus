add_library(integration_tests_lib OBJECT
    server-client_tests.cpp
)
target_include_directories(integration_tests_lib PUBLIC
    ${CMAKE_SOURCE_DIR}/tests
)
target_link_libraries(integration_tests_lib PUBLIC
    GTest::gtest 
    nlohmann_json::nlohmann_json
    config_scanner_lib
    file_inspector_lib
    socket_lib
    server_lib
    client_lib
)
target_compile_definitions(integration_tests_lib PRIVATE
    TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/config_files"
)