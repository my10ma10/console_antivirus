#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

#include "../defines.hpp"

using json = nlohmann::json;


// TODO: перейти на Ахо-Корасик
class ConfigScanner {
    std::vector<std::string> _patterns;
public:
    void loadPatterns(const fs::path &config_path);
    
    const std::vector<std::string>& getPatterns() const;
private:
    json parseFile(const fs::path &config_path);
};
