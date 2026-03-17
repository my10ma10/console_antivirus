#include <gtest/gtest.h>

#include <thread>
#include <chrono>
#include <functional>

#include "socket/socket.hpp"

static constexpr const char* PORT = "19999";

class SocketTest : public ::testing::Test {
protected:
    Socket server;
    Socket client;

    void SetUp() override
    {
        ASSERT_TRUE(server.bind(PORT));
        ASSERT_TRUE(server.listen(5));
    }

    void TearDown() override
    {
        client.close();
        server.close();
    }

    std::thread runServer(std::function<void(Socket&)> handler)
    {
        return std::thread([this, handler]() {
            auto accepted = server.accept();
            ASSERT_TRUE(accepted.has_value());
            handler(accepted.value());
        });
    }

    void connectClient()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        ASSERT_TRUE(client.connect(PORT, "127.0.0.1"));
    }
};


class SocketRawTest : public ::testing::Test {
protected:
    Socket s1;
    Socket s2;

    void TearDown() override
    {
        s1.close();
        s2.close();
    }
};

TEST_F(SocketRawTest, default_not_active)
{
    EXPECT_FALSE(s1.isActive());
}

TEST_F(SocketRawTest, after_bind_active)
{
    ASSERT_TRUE(s1.bind(PORT));
    EXPECT_TRUE(s1.isActive());
}

// connection tests

TEST_F(SocketRawTest, bind_same_port_twice)
{
    ASSERT_TRUE(s1.bind(PORT));
    ASSERT_TRUE(s1.listen(5));
    EXPECT_FALSE(s2.bind(PORT));
}

TEST_F(SocketRawTest, listen_without_bind)
{
    EXPECT_FALSE(s1.listen(5));
}

TEST_F(SocketRawTest, connect_to_non_existent_server)
{
    EXPECT_FALSE(s1.connect("19990", "127.0.0.1"));
}

TEST_F(SocketTest, connect_success)
{
    auto thr = runServer([](Socket&) {});

    connectClient();
    EXPECT_TRUE(client.isActive());

    thr.join();
}

// send/recv tests

TEST_F(SocketTest, send_recv_equal_data)
{
    auto t = runServer([](Socket& accepted) {
        auto msg = accepted.recv();

        ASSERT_TRUE(msg.has_value());
        EXPECT_EQ(msg.value(), "hello");

        accepted.send("world");
    });

    connectClient();
    client.send("hello");
    client.shutdownWrite();

    auto resp = client.recv();
    ASSERT_TRUE(resp.has_value());
    EXPECT_EQ(resp.value(), "world");

    t.join();
}

TEST_F(SocketTest, send_empty_string)
{
    auto t = runServer([](Socket& accepted) {
        auto msg = accepted.recv();

        ASSERT_TRUE(msg.has_value());
        EXPECT_EQ(msg.value(), "");
    });

    connectClient();
    client.send("");
    client.shutdownWrite();

    t.join();
}

TEST_F(SocketTest, send_large_data)
{
    std::string large(BUF_SIZE, 'z');

    auto t = runServer([&large](Socket& accepted) {
        auto msg = accepted.recv();

        ASSERT_TRUE(msg.has_value());
        EXPECT_EQ(msg.value(), large);
    });

    connectClient();
    client.send(large);
    client.shutdownWrite();

    t.join();
}

TEST_F(SocketRawTest, send_on_inactive_socket)
{
    EXPECT_FALSE(s1.send("hello").has_value());
}

TEST_F(SocketRawTest, recv_on_inactive_socket)
{
    EXPECT_FALSE(s1.recv().has_value());
}

// move semantic tests

TEST_F(SocketRawTest, move_constructor)
{
    ASSERT_TRUE(s1.bind(PORT));
    EXPECT_TRUE(s1.isActive());

    Socket s3 = std::move(s1);

    EXPECT_FALSE(s1.isActive());
    EXPECT_TRUE(s3.isActive());
}

TEST_F(SocketRawTest, move_assignment)
{
    ASSERT_TRUE(s1.bind(PORT));

    s2 = std::move(s1);

    EXPECT_FALSE(s1.isActive());
    EXPECT_TRUE(s2.isActive());
}

TEST_F(SocketRawTest, move_from_inactive)
{
    Socket s3 = std::move(s1);

    EXPECT_FALSE(s3.isActive());
}

TEST_F(SocketRawTest, close_becomes_inactive)
{
    ASSERT_TRUE(s1.bind(PORT));

    s1.close();

    EXPECT_FALSE(s1.isActive());
}

TEST_F(SocketRawTest, detach_becomes_inactive)
{
    ASSERT_TRUE(s1.bind(PORT));

    s1.detach();

    EXPECT_FALSE(s1.isActive());
}

TEST_F(SocketTest, poll_has_correct_events)
{
    struct pollfd pfd = server.pollfd(POLLIN);

    EXPECT_EQ(pfd.events, POLLIN);
    
    EXPECT_GE(pfd.fd, 0);
}

TEST_F(SocketTest, pollfd_trigger)
{
    struct pollfd pfd = server.pollfd(POLLIN);

    std::thread t([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        client.connect(PORT, "127.0.0.1");
    });

    int ret = poll(&pfd, 1, 2000);

    EXPECT_GT(ret, 0);
    EXPECT_TRUE(pfd.revents & POLLIN);

    t.join();
}