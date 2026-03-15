#include "stats_util.hpp"
#include <iostream>

int main(int, char*[]) {
    StatsUtil stats_util;

    stats_util.writeRequest();
    std::cout << stats_util.readResponse() << std::endl;

    return 0;
}
