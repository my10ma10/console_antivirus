#pragma once

#include <cstdint>
#include <filesystem>

namespace fs = std::filesystem;

inline constexpr std::size_t BUF_SIZE = 4096;
inline constexpr std::size_t BACKLOG = 5;

static constexpr const char* STATS_REQUEST_FIFO  = "/tmp/ca_to_stats.fifo";
static constexpr const char* STATS_RESPONSE_FIFO = "/tmp/ca_to_server.fifo";

static constexpr const char* GREEN_COLOR = "\033[32m";
static constexpr const char* RED_COLOR = "\033[31m";
static constexpr const char* RESET_COLOR = "\033[0m";