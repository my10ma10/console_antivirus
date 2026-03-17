#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <unistd.h>

#include "server.hpp"

class ServerTest : public ::testing::Test {
protected:
    Server server;

    const json& statJson() const { 
        return server._stat_json; 
    }

    void updateStats(const json& j) { 
        server.updateStats(j); 
    }

    void readChildrenStat(int fd) { 
        server.readChildrenStat(fd); 
    }

    void sendStatsToParent(InspectResult& res, int fd) {
        server.sendStatsToParent(res, fd);
    }

    json getStatJson(const Server &s) const {
        return s._stat_json;
    }
};

// constructor

TEST_F(ServerTest, defaultConstructor_Initializes_json_structure) {
    EXPECT_TRUE(statJson().contains("checked_files_count"));
    EXPECT_TRUE(statJson().contains("pattern_stat"));
    EXPECT_TRUE(statJson()["pattern_stat"].contains("found_count"));
    EXPECT_TRUE(statJson()["pattern_stat"].contains("patterns_types"));
}

TEST_F(ServerTest, defaultConstructor_Initializes_counters_to_zero) {
    EXPECT_EQ(statJson()["checked_files_count"], 0);
    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 0);
    EXPECT_TRUE(statJson()["pattern_stat"]["patterns_types"].empty());
}

// move tests

TEST_F(ServerTest, move_constructor__stat_json_is_transferred) {
    updateStats({{"found_patterns", {"pattern1"}}});

    Server moved(std::move(server));
    const json& moved_json = getStatJson(moved);

    EXPECT_EQ(moved_json["checked_files_count"], 1);
    EXPECT_EQ(moved_json["pattern_stat"]["found_count"], 1);
}

TEST_F(ServerTest, move_assignment__stat_json_is_transferred) {
    updateStats({{"found_patterns", {"pattern2", "pattern1"}}});

    Server other;
    other = std::move(server);

    EXPECT_EQ(getStatJson(other)["checked_files_count"], 1);
    EXPECT_EQ(getStatJson(other)["pattern_stat"]["found_count"], 2);
}

// updateStats

TEST_F(ServerTest, updateStats_increments_checked_files_count) {
    json insp = {{"found_patterns", json::array()}};

    updateStats(insp);
    EXPECT_EQ(statJson()["checked_files_count"], 1);

    updateStats(insp);
    EXPECT_EQ(statJson()["checked_files_count"], 2);
}

TEST_F(ServerTest, updateStats_no_patterns__does_not_found_count) {
    json insp = {{"found_patterns", json::array()}};

    updateStats(insp);

    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 0);
    EXPECT_TRUE(statJson()["pattern_stat"]["patterns_types"].empty());
}

TEST_F(ServerTest, updateStats_counts_found_patterns) {
    json insp = {{"found_patterns", {"test1", "test2"}}};

    updateStats(insp);

    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 2);
}

TEST_F(ServerTest, updateStats_tracks_pattern_types) {
    json insp = {{"found_patterns", {"test1", "test2"}}};

    updateStats(insp);

    const auto& types = statJson()["pattern_stat"]["patterns_types"];
    EXPECT_EQ(types["test1"], 1);
    EXPECT_EQ(types["test2"], 1);
}

TEST_F(ServerTest, updateStats_accumulates_pattern_types_across_calls) {
    updateStats({{"found_patterns", {"test1"}}});
    updateStats({{"found_patterns", {"test1", "test2"}}});

    const auto& types = statJson()["pattern_stat"]["patterns_types"];

    EXPECT_EQ(types["test1"], 2);
    EXPECT_EQ(types["test2"], 1);
    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 3);
}

// sendStatsToParent

TEST_F(ServerTest, sendStatsToParent_verified_result__writes_correct_json) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    InspectResult res{true, {"sqli", "xss"}};
    sendStatsToParent(res, pipefd[1]);
    close(pipefd[1]);

    char buf[512] = {};
    read(pipefd[0], buf, sizeof(buf));
    close(pipefd[0]);


    auto j = json::parse(buf);


    EXPECT_TRUE(j["verified"]);
    EXPECT_EQ(j["found_patterns"].size(), 2);
    EXPECT_EQ(j["found_patterns"][0], "sqli");
}

TEST_F(ServerTest, sendStatsToParent_not_verified__writes_false) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    InspectResult res{false, {}};
    sendStatsToParent(res, pipefd[1]);
    close(pipefd[1]);

    char buf[512] = {};
    read(pipefd[0], buf, sizeof(buf));
    close(pipefd[0]);


    auto j = json::parse(buf);


    EXPECT_FALSE(j["verified"]);
    EXPECT_TRUE(j["found_patterns"].empty());
}

TEST_F(ServerTest, sendStatsToParent_no_patterns__writes_empty_array) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    InspectResult res{true, {}};
    sendStatsToParent(res, pipefd[1]);
    close(pipefd[1]);

    char buf[512] = {};
    read(pipefd[0], buf, sizeof(buf));
    close(pipefd[0]);


    auto j = json::parse(buf);


    EXPECT_TRUE(j["found_patterns"].is_array());
    EXPECT_EQ(j["found_patterns"].size(), 0);
}

// readChildrenStat

TEST_F(ServerTest, readChildrenStat_parses_valid_json) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    json payload = {{"found_patterns", {"test1"}}};
    std::string s = payload.dump();
    write(pipefd[1], s.c_str(), s.size());

    readChildrenStat(pipefd[0]);

    EXPECT_EQ(statJson()["checked_files_count"], 1);
    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 1);

    close(pipefd[0]);
    close(pipefd[1]);
}

TEST_F(ServerTest, readChildrenStat_empty_pipe__does_nothing) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);
    close(pipefd[1]);

    readChildrenStat(pipefd[0]);
    close(pipefd[0]);

    EXPECT_EQ(statJson()["checked_files_count"], 0);
}

TEST_F(ServerTest, readChildrenStat_malformed_json__does_not_crash) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    const char* garbage = "not a json {{{{";
    write(pipefd[1], garbage, strlen(garbage));
    close(pipefd[1]);

    EXPECT_NO_THROW(readChildrenStat(pipefd[0]));
    close(pipefd[0]);
}

TEST_F(ServerTest, readChildrenStat_valid_json__updates_stats) {
    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    json payload = {{"found_patterns", {"pattern1", "pattern2"}}};
    std::string s = payload.dump();
    write(pipefd[1], s.c_str(), s.size());

    readChildrenStat(pipefd[0]);

    EXPECT_EQ(statJson()["checked_files_count"], 1);
    EXPECT_EQ(statJson()["pattern_stat"]["found_count"], 2);
    EXPECT_EQ(statJson()["pattern_stat"]["patterns_types"]["pattern1"], 1);
    EXPECT_EQ(statJson()["pattern_stat"]["patterns_types"]["pattern2"], 1);
    
    close(pipefd[1]);
    close(pipefd[0]);
}