#include "client.hpp"
#include <iostream>


int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Too few arguments to program\n";
        return 1;
    }
    
    fs::path checking_file(argv[1]);
    std::string port_str = argv[2];
    
    Client client(port_str, "127.0.0.1");

    client.sendFile(checking_file);
    if (client.isVerified()) {
        std::cout << "Файл прошёл проверку\n";
    }
    else {
        std::cout << "Файл не прошёл проверку и является вредоносным\n";
    }
    return 0;
} 

