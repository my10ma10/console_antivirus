add_library(socket_lib STATIC 
    socket/socket.hpp
    socket/socket.cpp
)

target_include_directories(socket_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(stats_util)
