
add_library (client_lib STATIC
    client.hpp
    client.cpp
)
target_link_libraries(client_lib PUBLIC 
    socket_lib
)

add_executable(client 
    main.cpp
)
target_link_libraries(client PUBLIC 
    client_lib
)
