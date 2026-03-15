add_executable(stats_util 
    stats_util.hpp
    stats_util.cpp
    main.cpp
)
target_link_libraries(stats_util PUBLIC
    nlohmann_json::nlohmann_json
)