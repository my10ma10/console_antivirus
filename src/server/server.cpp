#include "server.hpp"
#include <iostream>

Server::Server()
    : _inspector(_stat_json)
{
    initJson();
}

Server::Server(const fs::path &config_path)
    : _inspector(_stat_json, config_path)
{
    initJson();
}

void Server::connect(const std::string &port)
{
    if (!_socket.bind(port)) {
        throw std::runtime_error("Server bind error");
    }
    if (!_socket.listen(BACKLOG)) {
        throw std::runtime_error("Server listen error");
    }
}

void Server::serveClient()
{
    Socket client_socket = _socket.accept().value();

    auto file = client_socket.recv();
    
    bool verified = false;
    if (file.has_value()) {
        verified = _inspector.inspect(file.value());
    }

    client_socket.send(verified ? "1" : "0");
}

void Server::initJson()
{
    _stat_json["checked_files_count"] = 0;
    _stat_json["pattern_stat"] = json::object();
    _stat_json["pattern_stat"]["found_count"] = 0;
    _stat_json["pattern_stat"]["patterns_types"] = json::object();
}
