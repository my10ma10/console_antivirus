#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

#include <nlohmann/json.hpp>

#include "../defines.hpp"

using json = nlohmann::json;

class StatsUtil {
    int _request_fifo_fd = -1;
    int _response_fifo_fd = -1;
public:
    StatsUtil();
    ~StatsUtil();

    StatsUtil(const StatsUtil &other) = delete;
    StatsUtil &operator=(const StatsUtil &other) = delete;

    
    StatsUtil(StatsUtil &&other);
    StatsUtil &operator=(StatsUtil &&other);


    void writeRequest();
    std::string readResponse();

    void close();
    bool isActive() const;

private: 
    std::string responsePreparing(json j);
};