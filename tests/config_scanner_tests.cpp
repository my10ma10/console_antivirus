#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "server/config_scanner.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json; 


class ConfigScannerTest : public ::testing::Test {
protected:
    fs::path temp_dir;
    ConfigScanner scanner;

    void SetUp() override {
        temp_dir = fs::temp_directory_path() / fs::path("config_test_%%%%%%");
        fs::create_directories(temp_dir);
    }

    void TearDown() override {
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    fs::path createConfigFile(const std::string& filename, const std::string& content) {
        fs::path file_path = temp_dir / filename;
        std::ofstream out(file_path);

        out << content;
        out.close();
        
        return file_path;
    }
};


// happy tests

TEST_F(ConfigScannerTest, loadPatterns_valid_config) 
{
    std::string json_content = R"({ "patterns": ["*.log", "*.txt", "error_*"] })";
    fs::path config_path = createConfigFile("valid.json", json_content);

    scanner.loadPatterns(config_path);
    const auto& patterns = scanner.getPatterns();

    ASSERT_EQ(patterns.size(), 3);
    EXPECT_EQ(patterns[0], "*.log");
    EXPECT_EQ(patterns[1], "*.txt");
    EXPECT_EQ(patterns[2], "error_*");
}

TEST_F(ConfigScannerTest, loadPatterns_empty_patterns_array) 
{
    std::string json_content = R"({ "patterns": [] })";
    fs::path config_path = createConfigFile("empty_array.json", json_content);

    scanner.loadPatterns(config_path);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, getPatterns_init_state) 
{    
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}


// filesystem tests 

TEST_F(ConfigScannerTest, loadPatterns_file_not_found) 
{
    fs::path non_existent_path = temp_dir / "not_exists.json";

    EXPECT_THROW(scanner.loadPatterns(non_existent_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, loadPatterns_path_is_directory) 
{
    fs::path dir_path = temp_dir; 

    EXPECT_THROW(scanner.loadPatterns(dir_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, loadPatterns_invalid_json_syntax) 
{
    std::string json_content = "{ bad json }";
    fs::path config_path = createConfigFile("invalid.json", json_content);

    ASSERT_THROW(scanner.loadPatterns(config_path), std::exception);
}


// validate tests

TEST_F(ConfigScannerTest, loadPatterns_missing_patterns_key_in_json) 
{
    std::string json_content = R"({ "other_key": "value" })";
    fs::path config_path = createConfigFile("missing_key.json", json_content);

    EXPECT_THROW(scanner.loadPatterns(config_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, loadPatterns_patterns_not_array) 
{
    std::string json_content = R"({ "patterns": "not_an_array" })";
    fs::path config_path = createConfigFile("not_array.json", json_content);

    EXPECT_THROW(scanner.loadPatterns(config_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, loadPatterns_patterns_contains_not_string) 
{
    std::string json_content = R"({ "patterns": ["valid", 123, "another"] })";
    fs::path config_path = createConfigFile("mixed_types.json", json_content);

    EXPECT_THROW(scanner.loadPatterns(config_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

TEST_F(ConfigScannerTest, loadPatterns_null_value) 
{
    std::string json_content = "null";
    fs::path config_path = createConfigFile("null.json", json_content);

    EXPECT_THROW(scanner.loadPatterns(config_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

// param tests

class InvalidItemTypesTest : public ConfigScannerTest, 
                             public ::testing::WithParamInterface<std::string> 
{
};

TEST_P(InvalidItemTypesTest, loadPatterns_invalid_item_type_in_patterns) 
{
    std::string json_content = R"({ "patterns": [)" + GetParam() + R"(] })";
    fs::path config_path = createConfigFile("invalid.json", json_content);

    EXPECT_THROW(scanner.loadPatterns(config_path), std::runtime_error);
    const auto& patterns = scanner.getPatterns();

    EXPECT_TRUE(patterns.empty());
}

INSTANTIATE_TEST_SUITE_P(VariousTypes, InvalidItemTypesTest,
    ::testing::Values("123", "true", "null", "{}")
);