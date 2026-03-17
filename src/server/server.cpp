#include "server.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

#include <iostream>

Server::Server()
{
    initJson();
}

Server::Server(const fs::path &config_path)
    : _inspector(config_path)
{
    initJson();
}

Server::~Server()
{
    unlink(STATS_CHILDREN_FIFO);
    unlink(STATS_REQUEST_FIFO);
    unlink(STATS_RESPONSE_FIFO);
}

Server::Server(Server &&other)
{
    if (this != &other) {
        this->_socket = std::move(other._socket);
        this->_stat_json = other._stat_json;
        this->_inspector = other._inspector;
    }
}

Server &Server::operator=(Server &&other)
{
    if (this != &other) {
        this->_socket = std::move(other._socket);
        this->_stat_json = other._stat_json;
        this->_inspector = other._inspector;
    }
    return *this;
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

void Server::run(const std::atomic<bool>& running)
{
    makeFifos();

    int childen_fifo_fd = open(STATS_CHILDREN_FIFO, O_RDONLY | O_NONBLOCK);
    if (childen_fifo_fd == -1) {
        std::perror("open children fifo");
        return;
    }
    int req_fifo_fd = open(STATS_REQUEST_FIFO, O_RDONLY | O_NONBLOCK);
    if (req_fifo_fd == -1) {
        std::perror("open request fifo");
        return;
    }

    struct pollfd fds[3];
    fds[0] = _socket.pollfd(POLLIN); // TCP Socket - new client
    fds[1] = {childen_fifo_fd, POLLIN, 0}; // children stat
    fds[2] = {req_fifo_fd, POLLIN, 0}; // stat util requests

    while (running) {
        int ret = poll(fds, 3, 1000);
        if (ret == -1) {
            if (errno == EINTR) continue;
            std::perror("poll error");
            break;
        }
        else if (ret == 0) {
            // std::cout << "no events\n";
        }
        else {
            if (fds[0].revents & POLLIN) {
                auto client_socket = _socket.accept();
                if (client_socket.has_value()) {
                    handleClient(client_socket.value());
                }
            }
            if (fds[1].revents & POLLIN) {
                readChildrenStat(childen_fifo_fd);
            }
            if (fds[2].revents & POLLIN) {
                sendStatToUtil(req_fifo_fd);
            }
            
        }
    }
    while (wait(nullptr) > 0) {}

    ::close(childen_fifo_fd);
    ::close(req_fifo_fd);
}

void Server::handleClient(Socket &client_socket)
{
    pid_t pid = fork();
    if (pid == -1) {
        std::perror("fork error");
        std::exit(1);
    }
    else if (pid == 0) {
        _socket.detach();
        // std::cout << "clild: pid = " << getpid() << std::endl;
        report(client_socket);

        client_socket.close();
        _exit(0);
    }
    else {
        client_socket.detach();
        // std::cout << "parent: pid = " << getpid() << std::endl;
    }
}

void Server::report(Socket &client_socket)
{
    auto file = client_socket.recv();
    
    InspectResult result;
    if (file.has_value()) {
        result = _inspector.inspect(file.value());
    }
    
    client_socket.send(result.verified ? "1" : "0");


    int to_parent_fd = open(STATS_CHILDREN_FIFO, O_WRONLY);
    if (to_parent_fd == -1) {
        std::perror("open children stat fifo error");
        return;
    }
    
    sendStatsToParent(result, to_parent_fd);

    ::close(to_parent_fd);
}

void Server::sendStatsToParent(InspectResult &insp_res, int fifo_fd)
{
    json inspect_res_json;
    inspect_res_json["verified"] = insp_res.verified;
    inspect_res_json["found_patterns"] = insp_res.found_patterns;

    std::string res_str = inspect_res_json.dump();
    write(fifo_fd, res_str.c_str(), res_str.size());
}

void Server::readChildrenStat(int req_fifo_fd)
{
    std::vector<char> buf(BUF_SIZE);

    int read_n = read(req_fifo_fd, buf.data(), buf.capacity());
    if (read_n <= 0) {
        std::perror("read children stat error");
        return;
    }
    
    auto insp_result = json::parse(buf.data(), nullptr, false);

    if (insp_result.is_discarded()) {
        std::cerr << "inspect result is empty\n";
        return;
    }

    updateStats(insp_result);
}

void Server::sendStatToUtil(int req_fifo_fd)
{
    char req_buf[16];
    read(req_fifo_fd, req_buf, sizeof(req_buf));

    int resp_fifo_fd = open(STATS_RESPONSE_FIFO, O_WRONLY);
    if (resp_fifo_fd == -1) {
        std::perror("open response fifo error");
        return;
    }

    std::string stats_str = _stat_json.dump();
    write(resp_fifo_fd, stats_str.c_str(), stats_str.size());

    ::close(resp_fifo_fd);
}

void Server::updateStats(const json &inspect_stat)
{
    _stat_json["checked_files_count"] = _stat_json.value("checked_files_count", 0) + 1;
    auto& patterns_types = _stat_json["pattern_stat"]["patterns_types"];

    for (const auto &json_p : inspect_stat["found_patterns"]) {
        std::string pattern = json_p;
        patterns_types[pattern] = patterns_types.value(pattern, 0) + 1;
    }

    int insp_found = inspect_stat["found_patterns"].size();
    _stat_json["pattern_stat"]["found_count"] =
        _stat_json["pattern_stat"].value("found_count", 0) + insp_found;
}

void Server::makeFifos()
{
    mkfifo(STATS_CHILDREN_FIFO, 0644);
    mkfifo(STATS_REQUEST_FIFO, 0644);
    mkfifo(STATS_RESPONSE_FIFO, 0644);
}

void Server::initJson()
{
    _stat_json["checked_files_count"] = 0;
    _stat_json["pattern_stat"] = json::object();
    _stat_json["pattern_stat"]["found_count"] = 0;
    _stat_json["pattern_stat"]["patterns_types"] = json::object();
}
