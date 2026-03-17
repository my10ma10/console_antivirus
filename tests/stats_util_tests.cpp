#include <gtest/gtest.h>
#include "stats_util/stats_util.hpp"

class StatsUtilTest : public ::testing::Test {
protected:
    int req_pipe[2];
    int resp_pipe[2];

    StatsUtil makeUtil() {
        return StatsUtil(req_pipe[1], resp_pipe[0]);
    }

    std::string responsePreparing(const json& j) {
        return makeUtil().responsePreparing(j);
    }

    void SetUp() override {
        ASSERT_EQ(pipe(req_pipe), 0);
        ASSERT_EQ(pipe(resp_pipe), 0);
    }

    void TearDown() override {
        ::close(req_pipe[0]);
        ::close(resp_pipe[1]);
    }
};

// isActive

TEST_F(StatsUtilTest, isActive_valid_fds__returns_true) {
    auto util = makeUtil();

    EXPECT_TRUE(util.isActive());
}

TEST_F(StatsUtilTest, isActive_after_close__returns_false) {
    auto util = makeUtil();

    util.close();

    EXPECT_FALSE(util.isActive());
}

// move

TEST_F(StatsUtilTest, move_constructor__source_becomes_inactive) {
    auto util = makeUtil();
    StatsUtil moved(std::move(util));

    EXPECT_FALSE(util.isActive());
    EXPECT_TRUE(moved.isActive());
}

TEST_F(StatsUtilTest, move_assignment__source_becomes_inactive) {
    auto util = makeUtil();

    int req2[2], resp2[2];
    ASSERT_EQ(pipe(req2), 0);
    ASSERT_EQ(pipe(resp2), 0);
    StatsUtil other(req2[1], resp2[0]);

    other = std::move(util);

    EXPECT_FALSE(util.isActive());
    EXPECT_TRUE(other.isActive());

    ::close(req2[0]);
    ::close(resp2[1]);
}

// responsePreparing 

TEST_F(StatsUtilTest, responsePreparing_zero_stats__contains_zero_counts) {
    json j = {
        {"checked_files_count", 0},
        {"pattern_stat", {
            {"found_count", 0},
            {"patterns_types", json::object()}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find("0"), std::string::npos);
}

TEST_F(StatsUtilTest, responsePreparing_contains_checked_files_count) {
    json j = {
        {"checked_files_count", 42},
        {"pattern_stat", {
            {"found_count", 0},
            {"patterns_types", json::object()}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find("42"), std::string::npos);
}

TEST_F(StatsUtilTest, responsePreparing_contains_found_count) {
    json j = {
        {"checked_files_count", 1},
        {"pattern_stat", {
            {"found_count", 7},
            {"patterns_types", json::object()}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find("7"), std::string::npos);
}

TEST_F(StatsUtilTest, responsePreparing_contains_pattern_names) {
    json j = {
        {"checked_files_count", 2},
        {"pattern_stat", {
            {"found_count", 3},
            {"patterns_types", {{"pattern1", 2}, {"pattern2", 1}}}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find("pattern1"), std::string::npos);
    EXPECT_NE(result.find("pattern2"), std::string::npos);
}

TEST_F(StatsUtilTest, responsePreparing_singular_count__says_time) {
    json j = {
        {"checked_files_count", 1},
        {"pattern_stat", {
            {"found_count", 1},
            {"patterns_types", {{"pattern1", 1}}}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find(" time"), std::string::npos);
    EXPECT_EQ(result.find(" times"), std::string::npos);
}

TEST_F(StatsUtilTest, responsePreparing_plural_count__says_times) {
    json j = {
        {"checked_files_count", 5},
        {"pattern_stat", {
            {"found_count", 5},
            {"patterns_types", {{"pattern1", 5}}}
        }}
    };

    std::string result = responsePreparing(j);

    EXPECT_NE(result.find(" times"), std::string::npos);
}