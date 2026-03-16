#include "config_scanner.hpp"

#include <fstream>
#include <iostream>

void ConfigScanner::loadPatterns(const fs::path &config_path)
{
    json data = parseFile(config_path);

    if (!data.contains("patterns") || !data["patterns"].is_array()) {
        throw std::runtime_error("Patterns not found or is not an array");
    }
    for (const auto& item : data["patterns"]) {
        if (!item.is_string()) {
            throw std::runtime_error("Patterns contain non-string values");
        }
    }

    _patterns = data["patterns"];
}

const std::vector<std::string> &ConfigScanner::getPatterns() const
{
    return _patterns;
}

json ConfigScanner::parseFile(const fs::path &config_path)
{
    if (!fs::exists(config_path)) {
        std::cerr << "ConfigScanner: File not found\n";
        return {};
    }
    std::ifstream file_stream(config_path);

    if (!file_stream.is_open()) {
        std::cerr << "ConfigScanner: Cannot open file\n";
        return {};
    }

    return json::parse(file_stream);
}
