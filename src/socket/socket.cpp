#include "socket.hpp"


#include <stdexcept>
#include <iostream>
#include <vector>

Socket::Socket(int fd) : _socket_fd(fd)
{

}

Socket::~Socket()
{
    Socket::close();
}

Socket::Socket(Socket &&other)
{
    if (!other.isActive()) {
        return;
    }
    if (this != &other) {
        _addr_info = other._addr_info;
        _socket_fd = other._socket_fd;

        other._addr_info = nullptr;
        other._socket_fd = -1;
    }
}

Socket &Socket::operator=(Socket &&other)
{
    if (this != &other) {
        this->close();
        
        _addr_info = other._addr_info;
        _socket_fd = other._socket_fd;

        other._addr_info = nullptr;
        other._socket_fd = -1;
    }
    return *this;
}

bool Socket::connect(const std::string &port, const std::string &host)
{
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status = ::getaddrinfo(host.c_str(), port.c_str(), &hints, &_addr_info);
    if (status == -1) {
        std::cerr << "getaddrinfo error: " << ::gai_strerror(status) << std::endl;
        return false;
    }

    struct addrinfo *p;
    for (p = _addr_info; p != NULL; p = p->ai_next) {
        if (!createSocket(p)) {
            return false;
        }
        
        if (::connect(_socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            std::perror("connecting error");
            return false;
        }
    }
    return true;
}

bool Socket::bind(const std::string &port)
{
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = ::getaddrinfo(NULL, port.c_str(), &hints, &_addr_info);
    if (status == -1) {
        std::cerr << "getaddrinfo error: " << ::gai_strerror(status) << std::endl;
        return false;
    }

    struct addrinfo *p;
    for (p = _addr_info; p != NULL; p = p->ai_next) {
        if (!createSocket(p)) {
            continue;
        }    
    
        int opt = 1;
        if (::setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1) {
            std::perror("setsockopt error");
            return false;
        }

        if (::bind(_socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            std::perror("binding error");
            return false;            
        }
        break;
    }
    return true;
}

bool Socket::listen(int backlog) const
{
    if (::listen(_socket_fd, backlog) == -1) {
        std::perror("listening error");
        return false;
    }
    return true;
}

std::optional<Socket> Socket::accept() const {
    struct sockaddr_storage client_info;
    socklen_t info_size = sizeof(client_info);

    int client_fd = ::accept(_socket_fd, (struct sockaddr*)&client_info, &info_size);
    if (client_fd == -1) {
        std::perror("accept error");
        return std::nullopt;
    }
    return Socket(client_fd);
}

std::optional<ssize_t> Socket::send(const std::string &msg) const
{
    return Socket::send(msg.c_str(), msg.size());
}

std::optional<ssize_t> Socket::send(const void *data, const std::size_t size) const
{
    if (!isActive()) {
        return std::nullopt;
    }

    const char *buffer = static_cast<const char*>(data);
    std::size_t total_sent = 0;

    while (total_sent < size) {
        ssize_t sent = ::send(_socket_fd, buffer + total_sent, size - total_sent, 0);
        if (sent == -1) {
            std::perror("send error");
            return std::nullopt;
        }

        total_sent += sent;
    }
    return total_sent;
}

std::optional<std::string> Socket::recv()
{
    if (!isActive()) {
        return std::nullopt;
    }

    std::vector<char> recv_buf(BUF_SIZE, 0);
    std::size_t total_received = 0;

    while (total_received < BUF_SIZE) {
        ssize_t received = ::recv(_socket_fd, 
            recv_buf.data() + total_received, BUF_SIZE - total_received, 0);
        if (received == -1) {
            std::perror("recv error");
            return std::nullopt;
        }
        else if (received == 0) {
            break;
        }
        total_received += received;
    }
    return std::string(recv_buf.begin(), recv_buf.begin() + total_received);
}

void Socket::close()
{    
    if (isActive()) {     
        Socket::shutdown();   
        ::close(_socket_fd);
        _socket_fd = -1;
    }
    if (_addr_info) {
        freeaddrinfo(_addr_info);
        _addr_info = nullptr;
    }
}

void Socket::shutdown()
{
    if (::shutdown(_socket_fd, SHUT_RDWR) == -1) {

        if (errno != ENOTCONN) {
            std::perror("socket shutdown error");
        }
    }
}

void Socket::shutdownWrite()
{
    if (::shutdown(_socket_fd, SHUT_WR) == -1) {
        if (errno != ENOTCONN) {
            std::perror("socket shutdown _write error");
        }
    }
}

void Socket::detach()
{
    _socket_fd = -1;
    _addr_info = nullptr;
}

bool Socket::isActive() const
{
    return _socket_fd >= 0;
}

bool Socket::createSocket(struct addrinfo *p) 
{
    return ((_socket_fd = ::socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1);
}

