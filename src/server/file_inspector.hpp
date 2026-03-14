#pragma once
#include "../defines.hpp"
#include "config_scanner.hpp"

#include <string>

class FileInspector {
    ConfigScanner _scanner;
    json &_stat_json;
public:
    FileInspector(json &stat_json);
    FileInspector(json &stat_json, const fs::path &config_path);

    void inspect(const fs::path &filepath);

private:
    std::string readFile(
        const fs::path &filepath, 
        std::ios_base::openmode openmode
    );
};