#pragma once

#include "socket/socket.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

class ClientTest;

class Client {
    friend class ClientTest;
    
    Socket _socket;

public:
    Client() = default;

    void connect(const std::string &port, const std::string &ip_address);

    void sendFile(const fs::path &filepath);
    bool isVerified();

private:
    std::optional<std::ifstream> readFile(const fs::path &filepath);
};