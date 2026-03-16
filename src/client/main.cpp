#include "client.hpp"
#include <iostream>


// ./client filepath port
int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Too few arguments to program\n";
        return 1;
    }
    
    fs::path checking_file(argv[1]);
    std::string port_str = argv[2];
    
    try {
        Client client;
        client.connect(port_str, "127.0.0.1");

        client.sendFile(checking_file);
        
        if (client.isVerified()) {
            std::cout << "The file has been verified\n";
        }
        else {
            std::cout << "The file has not been verified and is considered malicious\n";
        }
        return 0;
    }
    catch (const std::runtime_error& err) {
        std::cout << "runtime error: " << err.what() << std::endl;
        return 1;
    }
    catch (const std::exception& err) {
        std::cout << "Unrecognized error: " << err.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cout << "Unrecognized error\n";
        return 1;
    }
} 

