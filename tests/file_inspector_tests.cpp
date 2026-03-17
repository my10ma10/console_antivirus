#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include "file_inspector.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json; 

class FileInspectorTest : public ::testing::Test {
protected:
    fs::path temp_dir;
    FileInspector inspector;

    void SetUp() override {
        temp_dir = fs::temp_directory_path() / fs::path("inspector_test_%%%%%%");
        fs::create_directories(temp_dir);
    }

    void TearDown() override {
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    fs::path createConfigFile(const std::string &filename, const std::string &content) {
        fs::path file_path = temp_dir / filename;
        std::ofstream out(file_path);

        out << content;
        out.close();

        return file_path;
    }

    void loadInspector(const std::vector<std::string> &patterns, const std::string &filename = "config.json") {
        json j;
        j["patterns"] = patterns;

        fs::path config = createConfigFile(filename, j.dump());

        inspector = FileInspector(config);
    }
};


// init tests

TEST_F(FileInspectorTest, default_constructor_produces_empty_scanner)
{
    InspectResult res = inspector.inspect("any content here");

    EXPECT_TRUE(res.verified);
    EXPECT_TRUE(res.found_patterns.empty());
}

TEST_F(FileInspectorTest, constructor_with_valid_config_does_not_throw)
{
    EXPECT_NO_THROW(loadInspector({"pattern1", "pattern2"}));
}

TEST_F(FileInspectorTest, constructor_with_invalid_config)
{
    fs::path missing = temp_dir / "no_such_file.txt";
    try {
        inspector = FileInspector(missing);

        InspectResult res = inspector.inspect("anything");

        EXPECT_TRUE(res.verified);
        EXPECT_TRUE(res.found_patterns.empty());
    } catch (const std::exception &) {
        SUCCEED();
    }
}

// inspect tests

TEST_F(FileInspectorTest, clean_content_returns_verified_true)
{
    loadInspector({"badword", "forbidden", "malware"});

    InspectResult res = inspector.inspect("This is a clean file");

    EXPECT_TRUE(res.verified);
    EXPECT_TRUE(res.found_patterns.empty());
}

TEST_F(FileInspectorTest, single_pattern_found_returns_verified_false)
{
    loadInspector({"badword", "forbidden", "malware"});

    InspectResult res = inspector.inspect("The file contains badword");

    EXPECT_FALSE(res.verified);
    ASSERT_EQ(res.found_patterns.size(), 1);
    EXPECT_EQ(res.found_patterns[0], "badword");
}

TEST_F(FileInspectorTest, all_patterns_found_returns_all_in_result)
{
    loadInspector({"badword", "forbidden", "malware"});

    InspectResult res = inspector.inspect("badword forbidden malware all here");

    EXPECT_FALSE(res.verified);
    ASSERT_EQ(res.found_patterns.size(), 3);
    EXPECT_EQ(res.found_patterns[0], "badword");
    EXPECT_EQ(res.found_patterns[1], "forbidden");
    EXPECT_EQ(res.found_patterns[2], "malware");
}

TEST_F(FileInspectorTest, repeated_pattern_counted_once)
{
    loadInspector({"badword", "forbidden", "malware"});

    InspectResult res = inspector.inspect("badword badword badword");

    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns.size(), 1);
    EXPECT_EQ(res.found_patterns[0], "badword");
}

// boundary cases

TEST_F(FileInspectorTest, empty_content_returns_verified_true)
{
    loadInspector({"hello", "world"});

    InspectResult res = inspector.inspect("");

    EXPECT_TRUE(res.verified);
    EXPECT_TRUE(res.found_patterns.empty());
}

TEST_F(FileInspectorTest, pattern_at_beginning)
{
    loadInspector({"hello", "world"});

    InspectResult res = inspector.inspect("hello go go go");

    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns[0], "hello");
}

TEST_F(FileInspectorTest, pattern_at_end)
{
    loadInspector({"hello", "world"});
    
    InspectResult res = inspector.inspect("the end of the world");

    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns[0], "world");
}

TEST_F(FileInspectorTest, pattern_as_substring)
{
    loadInspector({"hello", "world"});

    InspectResult res = inspector.inspect("hellooo");

    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns.size(), 1);
    EXPECT_EQ(res.found_patterns[0], "hello");
}

TEST_F(FileInspectorTest, case_sensitive_no_match)
{
    loadInspector({"hello", "world"});

    InspectResult res = inspector.inspect("Hello World");

    EXPECT_TRUE(res.verified);
    EXPECT_TRUE(res.found_patterns.empty());
}

TEST_F(FileInspectorTest, pattern_with_spaces_no_match)
{
    loadInspector({"bad word"});

    InspectResult res = inspector.inspect("this has badword inside");

    EXPECT_TRUE(res.verified);
    EXPECT_TRUE(res.found_patterns.empty());
}

TEST_F(FileInspectorTest, pattern_with_spec_chars)
{
    loadInspector({"<script>", "DROP TABLE"});

    InspectResult res = inspector.inspect("alert! <script> DROP TABLE SomeTable;");

    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns.size(), 2);
}

TEST_F(FileInspectorTest, long_pattern)
{
    std::string long_pat(1000, 'a');
    loadInspector({long_pat});

    EXPECT_FALSE(inspector.inspect("prefix " + long_pat + " suffix").verified);
    EXPECT_TRUE(inspector.inspect("prefix " + std::string(999, 'a') + " suffix").verified);
}

TEST_F(FileInspectorTest, large_content)
{
    loadInspector({"needle"});
    std::string large(1'000'000, 'x');
    EXPECT_TRUE(inspector.inspect(large).verified);

    large += "needle";
    InspectResult res = inspector.inspect(large);
    EXPECT_FALSE(res.verified);
    EXPECT_EQ(res.found_patterns[0], "needle");
}

// config_tests

TEST_F(FileInspectorTest, empty_config_always_cerified)
{
    loadInspector({});

    EXPECT_TRUE(inspector.inspect("").verified);
    EXPECT_TRUE(inspector.inspect("anything").verified);
    EXPECT_TRUE(inspector.inspect("badword forbidden").verified);
}

// reuse tests

TEST_F(FileInspectorTest, multiple_calls_are_independent)
{
    loadInspector({"secret"});

    InspectResult r1 = inspector.inspect("no match here");
    EXPECT_TRUE(r1.verified);
    EXPECT_TRUE(r1.found_patterns.empty());

    InspectResult r2 = inspector.inspect("file has secret inside");
    EXPECT_FALSE(r2.verified);
    EXPECT_EQ(r2.found_patterns.size(), 1);

    InspectResult r3 = inspector.inspect("no match again");
    EXPECT_TRUE(r3.verified);
    EXPECT_TRUE(r3.found_patterns.empty());
}