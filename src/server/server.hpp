#pragma once

#include "socket/socket.hpp"
#include "file_inspector.hpp"

#include <nlohmann/json.hpp>

class Server {
    Socket _socket;

    FileInspector _inspector;
    json _stat_json;
public:
    Server();
    explicit Server(const fs::path &config_path);
    
    void connect(const std::string &port, const std::string &ip_address);
    
};