
add_library(config_scanner_lib STATIC
    config_scanner.cpp
    config_scanner.hpp
)
target_link_libraries(config_scanner_lib PUBLIC
    nlohmann_json::nlohmann_json
)
target_include_directories(config_scanner_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)


add_library(file_inspector_lib STATIC
    file_inspector.cpp
    file_inspector.hpp
)
target_link_libraries(file_inspector_lib PUBLIC
    config_scanner_lib
)
target_include_directories(file_inspector_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)



add_executable(server 
    server.hpp
    server.cpp
    main.cpp
)
target_link_libraries(server PUBLIC
    socket_lib
    file_inspector_lib
)