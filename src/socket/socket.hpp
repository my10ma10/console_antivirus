#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <cstring>

#include <unistd.h>

#include <optional>
#include <string>

#include "../defines.hpp"

class Socket {
    int _socket_fd = -1;

    struct addrinfo *_addr_info = nullptr;

public:
    Socket() = default;
    Socket(int fd);
    ~Socket();

    Socket(const Socket&) = delete;
    Socket &operator=(const Socket&) = delete;

    Socket(Socket &&other);
    Socket &operator=(Socket &&other);

    bool connect(const std::string &port, const std::string &host);
    bool bind(const std::string &port);
    bool listen(int backlog) const;

    std::optional<Socket> accept() const;

    std::optional<ssize_t> send(const std::string &msg) const;
    std::optional<ssize_t> send(const void *data, const std::size_t size) const;
    std::optional<std::string> recv();

    void close();
    void shutdown();

    bool isActive() const;

private:
    bool createSocket(struct addrinfo *p);

};