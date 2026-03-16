#pragma once

#include "socket/socket.hpp"
#include "file_inspector.hpp"

#include <nlohmann/json.hpp>
#include <atomic>

class Server {
    Socket _socket;

    json _stat_json;
    FileInspector _inspector;
    
public:
    Server();
    explicit Server(const fs::path &config_path);

    ~Server();

    Server(const Server &other) = delete;
    Server &operator=(const Server &other) = delete;

    Server(Server &&other);
    Server &operator=(Server &&other);
    
    void connect(const std::string &port);

    void run(const std::atomic<bool>& running);


private: 
    void initJson();
    void makeFifos();
    void handleClient(Socket &client_socket);
    void report(Socket &client_socket);
    void sendStatsToParent(InspectResult &insp_res);

    void readChildrenStat(int reqs_fifo_fd);
    void sendStatToUtil(int reqs_fifo_fd);
};