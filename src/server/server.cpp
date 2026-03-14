#include "server.hpp"

Server::Server()
    : _inspector(_stat_json)
{
}

Server::Server(const fs::path &config_path)
    : _inspector(_stat_json, config_path)
{
}

void Server::connect(const std::string &port, const std::string &ip_address)
{
    if (!_socket.bind(port)) {
        throw std::runtime_error("Server bind error");
    }
    if (!_socket.listen(BACKLOG)) {
        throw std::runtime_error("Server listen error");
    }
    Socket listen_socket = _socket.accept().value();
}
