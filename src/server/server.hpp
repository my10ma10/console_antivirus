#pragma once

#include "socket/socket.hpp"
#include "file_inspector.hpp"

#include <nlohmann/json.hpp>

class Server {
    Socket _socket;

    json _stat_json;
    FileInspector _inspector;
public:
    Server();
    explicit Server(const fs::path &config_path);
    
    void connect(const std::string &port);

    void handleClient();

private: 
    void initJson();
    void report(Socket &client_socket);
};