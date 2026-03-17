add_executable(tests
    config_scanner_tests.cpp
    file_inspector_tests.cpp
    socket_tests.cpp
    main.cpp
)


target_include_directories(tests PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(tests PRIVATE
    GTest::gtest 
    config_scanner_lib
    file_inspector_lib
    socket_lib
    nlohmann_json::nlohmann_json
)

target_compile_options(tests PRIVATE --coverage -O0 -g)
target_link_options(tests PRIVATE --coverage)

include(GoogleTest)