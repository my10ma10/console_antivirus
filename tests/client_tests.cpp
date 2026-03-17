#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

#include "client/client.hpp"

namespace fs = std::filesystem;

class ClientTest : public ::testing::Test {
protected:
    Client client;
    
    fs::path tmp_path;

    void SetUp() override {
        tmp_path = fs::temp_directory_path() / "client_test_file.txt";
    }

    void TearDown() override {
        fs::remove(tmp_path);
    }

    void createFile(const std::string& content) {
        std::ofstream f(tmp_path);
        f << content;
    }

    std::optional<std::ifstream> ClientreadFile(const fs::path& path) {
        return client.readFile(path);
    }
};

// readFile

TEST_F(ClientTest, readFile_existing_file__returns_stream) {
    createFile("some content");

    auto result = ClientreadFile(tmp_path);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result->is_open());
}

TEST_F(ClientTest, readFile_existing_file__stream_contains_correct_content) {
    createFile("hello world");

    auto result = ClientreadFile(tmp_path);
    ASSERT_TRUE(result.has_value());

    std::string line;
    std::getline(result.value(), line);
    EXPECT_EQ(line, "hello world");
}

TEST_F(ClientTest, readFile_nonexistent_file__returns_nullopt) {
    auto result = ClientreadFile("/nonexistent/path/file.txt");

    EXPECT_FALSE(result.has_value());
}

TEST_F(ClientTest, readFile_empty_file__returns_open_stream) {
    createFile("");

    auto result = ClientreadFile(tmp_path);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result->is_open());
}