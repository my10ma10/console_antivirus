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

void Server::handleClient()
{
    Socket client_socket = _socket.accept().value();

    pid_t pid = fork();
    if (pid == -1) {
        std::perror("fork error");
        std::exit(1);
    }
    else if (pid == 0) {
        _socket.detach();
        std::cout << "clild: pid = " << getpid() << std::endl;
        report(client_socket);

        client_socket.close();
        _exit(0);
    }
    else {
        client_socket.detach();
        std::cout << "parent: pid = " << getpid() << std::endl;
    }
}

void Server::report(Socket &client_socket)
{
    auto file = client_socket.recv();
    
    bool verified = false;
    if (file.has_value()) {
        verified = _inspector.inspect(file.value());
    }
    std::cout << std::boolalpha << verified << std::endl;
    client_socket.send(verified ? "1" : "0");
}

void Server::initJson()
{
    _stat_json["checked_files_count"] = 0;
    _stat_json["pattern_stat"] = json::object();
    _stat_json["pattern_stat"]["found_count"] = 0;
    _stat_json["pattern_stat"]["patterns_types"] = json::object();
}
