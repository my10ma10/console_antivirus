#include "stats_util.hpp"
#include <iostream>

int main(int, char*[]) {
    try {
        StatsUtil stats_util;

        stats_util.writeRequest();
        usleep(1000);
        std::cout << stats_util.readResponse() << std::endl;
        
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
} 

