#include "file_inspector.hpp"

#include <fstream>
#include <iostream>
#include <iterator>

FileInspector::FileInspector(const fs::path &config_path)
{
    _scanner.loadPatterns(config_path);
}

InspectResult FileInspector::inspect(const std::string &file_content)
{
    InspectResult res;
    res.verified = true;

    for (const std::string &pattern : _scanner.getPatterns()) {
        if (file_content.find(pattern) == std::string::npos) {
            continue;
        }

        res.found_patterns.emplace_back(pattern);
        res.verified = false;
    }
    return res;
}
