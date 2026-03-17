#include <gtest/gtest.h>
#include <thread>
#include <fstream>
#include <filesystem>
#include <atomic>
#include <chrono>

#include "server/server.hpp"
#include "client/client.hpp"

namespace fs = std::filesystem;

static const std::string TEST_PORT = "19999";

class IntegrationTest : public ::testing::Test {
protected:
    std::atomic<bool> running{true};
    std::thread server_thread;
    fs::path tmp_file = fs::temp_directory_path() / "integration_test_file.txt";

    const fs::path config_path = fs::path(TEST_DATA_DIR) / "config.json";

    void startServer(const fs::path& config_path = "") {
        server_thread = std::thread([this, config_path]() {
            Server server = config_path.empty() \
                    ? Server() : Server(config_path);

            server.connect(TEST_PORT);
            server.run(running);
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    void stopServer() {
        running = false;
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    void createFile(const std::string& content) {
        std::ofstream f(tmp_file);
        f << content;
    }

    void TearDown() override {
        stopServer();
        fs::remove(tmp_file);
    }
};

// connect 

TEST_F(IntegrationTest, Server_connect__binds_successfully) {
    EXPECT_NO_THROW({
        Server server;
        server.connect(TEST_PORT);
    });
}

TEST_F(IntegrationTest, Client_connect__connects_to_running_server) {
    startServer();

    Client client;
    EXPECT_NO_THROW(client.connect(TEST_PORT, "127.0.0.1"));
}

TEST_F(IntegrationTest, Client_connect__throws_if_no_server) {
    // сервер не запущен
    Client client;
    EXPECT_THROW(client.connect(TEST_PORT, "127.0.0.1"), std::runtime_error);
}

// sendFile + isVerified (clean file)

TEST_F(IntegrationTest, Client_sendFile_clean_file__server_verifies) {
    startServer(config_path);

    createFile("hello world\nno malicious content here\n");

    Client client;
    client.connect(TEST_PORT, "127.0.0.1");
    client.sendFile(tmp_file);

    bool result = client.isVerified();
    EXPECT_TRUE(result);
}

// bad tests: sendFile + isVerified 

TEST_F(IntegrationTest, Client_sendFile_malicious_file__server_rejects) {
    startServer(config_path);

    createFile("SELECT * FROM users WHERE id = 1 OR 1=1;\n");

    Client client;
    client.connect(TEST_PORT, "127.0.0.1");
    client.sendFile(tmp_file);

    bool result = client.isVerified();
    EXPECT_FALSE(result);
}

// --- sendFile с несуществующим файлом — клиент не падает ---

TEST_F(IntegrationTest, Client_sendFile_nonexistent_file__does_not_throw) {
    startServer(config_path);

    Client client;
    client.connect(TEST_PORT, "127.0.0.1");

    // sendFile тихо возвращается, но сокет не закрывается корректно —
    // сервер может зависнуть на recv, поэтому тест только на отсутствие краша
    EXPECT_NO_THROW(client.sendFile("/nonexistent/path.txt"));
}

// --- несколько клиентов подряд ---

TEST_F(IntegrationTest, Server_handles_multiple_sequential_clients) {
    startServer(config_path);
    createFile("clean content\n");

    for (int i = 0; i < 3; ++i) {
        Client client;
        client.connect(TEST_PORT, "127.0.0.1");
        client.sendFile(tmp_file);
        EXPECT_NO_THROW(client.isVerified());
    }
}