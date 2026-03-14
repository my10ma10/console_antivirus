#include "client.hpp"

void Client::connect(const std::string &port, const std::string &ip_address)
{
    if (!_socket.connect(port, ip_address)) {
        throw std::runtime_error("Client connection error");
    }
}

void Client::sendFile(const fs::path &filepath)
{
    auto file_stream = readFile(filepath);
    if (!file_stream.has_value()) {
        return;
    }
    
    std::string line;
    while (std::getline(file_stream.value(), line)) {
        _socket.send(line);
    }

    file_stream.value().close();
}

bool Client::isVerified()
{
    auto verif_res = _socket.recv();
    if (!verif_res.has_value()) {
        std::cerr << "Getting verification resutl error\n";
        return false;
    }

    return verif_res == "0";
}

std::optional<std::ifstream> Client::readFile(const fs::path &filepath)
{
    std::cout << filepath << std::endl;
    if (!fs::exists(filepath)) {
        std::cerr << "Client: File not found\n";
        return std::nullopt;
    }

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Client: Cannot open file\n";
        return std::nullopt;
    }

    return file;
}
