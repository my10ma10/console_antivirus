add_executable(client 
    client.hpp
    client.cpp
    main.cpp
)

target_link_libraries(client PUBLIC 
    socket_lib
)
