#include "server.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Too few arguments to program\n";
        return 1;
    }
    
    fs::path config_file(argv[1]);
    std::string port_str = argv[2];
    
    try {
        Server server(config_file);

        server.connect(port_str);
        while (true) {
            server.serveClient();
        }
        return 0;

    }
    catch (const std::runtime_error& err) {
        std::cout << "runtime error: " << err.what() << std::endl;
    }
    catch (const std::exception& err) {
        std::cout << "Unrecognized error: " << err.what() << std::endl;
    }
} 

