#include "file_inspector.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

FileInspector::FileInspector(json &stat_json)
    : _stat_json(stat_json)
{
}

FileInspector::FileInspector(json &stat_json, const fs::path &config_path)
    : _stat_json(stat_json)
{
    _scanner.loadPatterns(config_path);
}

void FileInspector::inspect(const fs::path &filepath)
{
    std::string content = readFile(filepath, std::ios::binary);
    for (const std::string &pattern : _scanner.getPatterns()) {
        if (content.find(pattern) == std::string::npos) {
            return;
        }
        _stat_json["pattern_stat"]["patterns_types"][pattern] = \
            _stat_json["pattern_stat"]["patterns_types"].value(pattern, 0) + 1;
        
    }
}

std::string FileInspector::readFile(
    const fs::path &filepath, 
    std::ios_base::openmode openmode)
{
    if (!fs::exists(filepath)) {
        std::cerr << "FileInspector: File not found\n";
        return "";
    }

    std::ifstream file(filepath, openmode);
    if (!file.is_open()) {
        std::cerr << "FileInspector: Cannot open file\n";
        return "";
    }
    
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
