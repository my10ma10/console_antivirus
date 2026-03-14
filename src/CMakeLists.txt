add_library(socket_lib STATIC 
    socket/socket.hpp
    socket/socket.cpp
)

add_subdirectory(server)
add_subdirectory(client)
