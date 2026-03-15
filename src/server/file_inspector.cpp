#include "file_inspector.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

FileInspector::FileInspector(json &stat_json)
    : _stat_json(stat_json)
{
    init();
}

FileInspector::FileInspector(json &stat_json, const fs::path &config_path)
    : _stat_json(stat_json)
{
    init();
    _scanner.loadPatterns(config_path);
}

bool FileInspector::inspect(const std::string &file_content)
{
    _stat_json["checked_files_count"] = _stat_json.value("checked_files_count", 0) + 1;
    
    bool verified = true;
    auto& patterns_types = _stat_json["pattern_stat"]["patterns_types"];

    for (const std::string &pattern : _scanner.getPatterns()) {
        if (file_content.find(pattern) == std::string::npos) {
            continue;
        }
        patterns_types[pattern] = patterns_types.value(pattern, 0) + 1;
        verified = false;
    }

    // std::clog << _stat_json << std::endl;
    return verified;
}

void FileInspector::init()
{
}

// std::string FileInspector::readFile(
//     const fs::path &filepath, 
//     std::ios_base::openmode openmode)
// {
//     if (!fs::exists(filepath)) {
//         std::cerr << "FileInspector: File not found\n";
//         return "";
//     }

//     std::ifstream file(filepath, openmode);
//     if (!file.is_open()) {
//         std::cerr << "FileInspector: Cannot open file\n";
//         return "";
//     }
    
//     return std::string(
//         std::istreambuf_iterator<char>(file),
//         std::istreambuf_iterator<char>()
//     );
// }
