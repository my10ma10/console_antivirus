#pragma once

#include "socket/socket.hpp"

// #include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>

class Client {
    Socket _socket;

    // std::atomic<bool> _is_active{true};
    
public:
    Client() = default;

    void connect(const std::string &port, const std::string &ip_address);
    // void stop();

    void sendFile(const fs::path &filepath);
    bool isVerified();

private:
    std::optional<std::ifstream> readFile(const fs::path &filepath);
};