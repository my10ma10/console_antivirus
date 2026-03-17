#include "stats_util.hpp"
#include <fcntl.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

StatsUtil::StatsUtil(int req_fd, int resp_fd)
    : _request_fifo_fd(req_fd), _response_fifo_fd(resp_fd) 
{
}

StatsUtil::StatsUtil()
{
    this->_response_fifo_fd = open(STATS_RESPONSE_FIFO, O_RDONLY | O_NONBLOCK);
    this->_request_fifo_fd = open(STATS_REQUEST_FIFO, O_WRONLY);

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
        this->_response_fifo_fd = other._response_fifo_fd;
        other._response_fifo_fd = -1;

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
        this->_response_fifo_fd = other._response_fifo_fd;
        other._response_fifo_fd = -1;

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
    std::vector<char> stat_buf(BUF_SIZE);
    
    ssize_t read_n = read(_response_fifo_fd, stat_buf.data(), stat_buf.size());
    if (read_n == -1) {
        if (errno == EAGAIN) {
            std::cerr << "No data yet\n";
            return "";
        }
        perror("read error");
        throw std::runtime_error("StatsUtil read error");        
    }


    std::string stat_str(stat_buf.data(), read_n);
    auto resp = json::parse(stat_str);
    
    return responsePreparing(resp);
}

void StatsUtil::close()
{
    if (isActive()) {
        ::close(_request_fifo_fd);
        _request_fifo_fd = -1;

        ::close(_response_fifo_fd);
        _response_fifo_fd = -1;
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
        "Number of files checked: " << GREEN_COLOR << j["checked_files_count"] << RESET_COLOR << \
        "\nNumber of malware patterns: " << RED_COLOR << j["pattern_stat"]["found_count"] << RESET_COLOR << \
        "\nPatterns: ";
    
    for (auto& [pattern, count] : j["pattern_stat"]["patterns_types"].items()) {
        ss << "\n\t" << pattern << ": " 
            << RED_COLOR << count << RESET_COLOR << (count == 1 ? " time" : " times");
    }
    return ss.str();
}
