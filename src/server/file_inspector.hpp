#pragma once
#include "../defines.hpp"
#include "config_scanner.hpp"

#include <string>
#include <vector>

struct InspectResult {
    bool verified;
    std::vector<std::string> found_patterns;
};

class FileInspector {
    ConfigScanner _scanner;

public:

    FileInspector() = default;
    FileInspector(const fs::path &config_path);

    InspectResult inspect(const std::string &file_content);

    // std::string readFile(
    //     const fs::path &filepath, 
    //     std::ios_base::openmode openmode
    // );
};