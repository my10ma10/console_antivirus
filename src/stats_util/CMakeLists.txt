add_library(stats_util_lib STATIC
    stats_util.hpp
    stats_util.cpp
)
target_link_libraries(stats_util_lib PUBLIC
    nlohmann_json::nlohmann_json
)
target_include_directories(stats_util_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(stats_util 
    main.cpp
)
target_link_libraries(stats_util PUBLIC
    stats_util_lib
)