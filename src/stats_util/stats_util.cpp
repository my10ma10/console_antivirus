#include "stats_util.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <sstream>

StatsUtil::StatsUtil()
{
    if (mkfifo(STATS_REQUEST_FIFO, 0644) == -1) {
        std::perror("mkfifo error");
        throw std::runtime_error("StatsUtil mkfifo error");
    }
    this->_request_fifo_fd = open(STATS_REQUEST_FIFO, O_WRONLY);
    this->_response_fifo_fd = open(STATS_RESPONSE_FIFO, O_RDONLY);

    if (!isActive()) {
        perror("open error");
        throw std::runtime_error("StatsUtil open error");
    }
}

StatsUtil::~StatsUtil()
{
    StatsUtil::close();
}

StatsUtil::StatsUtil(StatsUtil &&other)
{
    if (!other.isActive()) {
        return;
    }
    if (this != &other) {
        this->_request_fifo_fd = other._request_fifo_fd;
        other._request_fifo_fd = -1;
    }
}

StatsUtil &StatsUtil::operator=(StatsUtil &&other)
{
    if (this != &other) {
        if (isActive()) {
            StatsUtil::close();
        }
        this->_request_fifo_fd = other._request_fifo_fd;
        other._request_fifo_fd = -1;
    }
    return *this;
}

void StatsUtil::writeRequest()
{
    std::string buf("GET_STATS");
    if (write(_request_fifo_fd, buf.data(), buf.size() + 1) == -1) {
        perror("write error");
        throw std::runtime_error("StatsUtil write error");
    }
}

std::string StatsUtil::readResponse()
{
    std::string str_stat;
    str_stat.reserve(BUF_SIZE);
    
    if (read(_response_fifo_fd, str_stat.data(), BUF_SIZE) == -1) {
        perror("read error");
        throw std::runtime_error("StatsUtil read error");        
    }
    
    return responsePreparing(json::parse(str_stat));
}

void StatsUtil::close()
{
    if (isActive()) {
        ::close(_request_fifo_fd);
        _request_fifo_fd = -1;
        unlink(STATS_REQUEST_FIFO);
    }
}

bool StatsUtil::isActive() const
{
    return _request_fifo_fd >= 0 && _response_fifo_fd >= 0;
}

std::string StatsUtil::responsePreparing(json j)
{
    std::stringstream ss;

    ss <<   
        "Stats: \n" << \
        "Number of files checked: " << j["checked_files_count"] << \
        "\nNumber of malware patterns: " << RED_COLOR << j["pattern_stat"]["found_count"] << RESET_COLOR << \
        "\nPatterns: ";
    
    for (auto& [pattern, count] : j["pattern_stat"]["patterns_types"].items()) {
        ss << "\n\t" << pattern << ": " 
            << RED_COLOR << count << RESET_COLOR << (count == 1 ? " time" : " times");
    }
    return ss.str();
}
