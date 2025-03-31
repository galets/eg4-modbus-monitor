#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <signal.h>
#include <unistd.h>
#include "../src/modbus.hpp"
#include "../src/modbusDeviceManager.hpp"
#include "../src/registers.hpp"
#include "../src/mqtt.hpp"
#include "../src/hass.hpp"
#include "../src/utils.hpp"

class MqttTest : public ::testing::Test {
protected:
    int pid = 0;
    void SetUp() override {
        pid = fork();
        if (pid == 0) {
            system("mosquitto -p 52871 2>/dev/null");
            exit(0);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cerr << "mosquitto server PID=" << pid << std::endl;
    }

    void Reset() {
        std::cerr << "resetting connection" << std::endl;
        TearDown();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        SetUp();
    }

    void TearDown() override {
        if (pid > 0) {
            kill(pid, SIGTERM);
        }
    }
};


TEST_F(MqttTest, TestSub) {
    Mqtt mqtt("tcp://127.0.0.1:52871", "", "", "test-client");
    std::string received;
    mqtt.listen("topics/test1", [&received](const std::string &msg) {
        received = msg;
    });
    ASSERT_EQ(received, "");
    system("mosquitto_pub -L mqtt://127.0.0.1:52871/topics/test1 -m abc");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(received, "abc");
    system("mosquitto_pub -L mqtt://127.0.0.1:52871/topics/test1 -m 123");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(received, "123");
}

TEST_F(MqttTest, TestReconnect) {
    Mqtt mqtt("tcp://127.0.0.1:52871", "", "", "test-client");
    std::string received;
    mqtt.listen("topics/test1", [&received](const std::string &msg) {
        received = msg;
    });
    ASSERT_EQ(received, "");
    system("mosquitto_pub -L mqtt://127.0.0.1:52871/topics/test1 -m abc");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(received, "abc");

    Reset();
    std::this_thread::sleep_for(std::chrono::seconds(5));

    system("mosquitto_pub -L mqtt://127.0.0.1:52871/topics/test1 -m 123");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_EQ(received, "123");
}
