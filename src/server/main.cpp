#include "server.hpp"
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <atomic>


void sigactionPreparing();

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Too few arguments to program\n";
        return 1;
    }
    
    fs::path config_file(argv[1]);
    std::string port_str = argv[2];

    static std::atomic<bool> g_running{true};

    sigactionPreparing();
    
    signal(SIGTERM, [](int) { 
        g_running = false; 
        std::exit(0);
    });
    signal(SIGINT, [](int) { 
        g_running = false;
        std::exit(0);
    });

    try {
        Server server(config_file);

        server.connect(port_str);
        while (g_running) {
            server.handleClient();
            if (g_running == false) {
                std::exit(0);
            }
        }
        std::cout << "Waiting all children\n";
        return 0;
    }
    catch (const std::runtime_error& err) {
        std::cout << "runtime error: " << err.what() << std::endl;
    }
    catch (const std::exception& err) {
        std::cout << "Unrecognized error: " << err.what() << std::endl;
    }
} 

void sigchldHandler(int);

void sigactionPreparing() {
    struct sigaction sa{};
    sa.sa_handler = sigchldHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, nullptr) == -1) {
        std::perror("sigaction");
        std::exit(1);
    }
}


void sigchldHandler(int) {
    while(waitpid(-1, nullptr, WNOHANG) > 0) {}
}