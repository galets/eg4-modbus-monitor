#pragma once

#include <string>
#include <iostream>
#include <mqtt/async_client.h>

// requires libpaho-mqttpp-dev and libpaho-mqtt-dev

class Mqtt {
  
public:
    Mqtt() : Mqtt(envOrDefault("MQTT_URI", "tcp://localhost:1883"),
        envOrDefault("MQTT_USERNAME", ""), 
        envOrDefault("MQTT_PASSWORD", ""), 
        envOrDefault("MQTT_CLIENT_ID", "modbus-local-reader")) {
    }

    Mqtt(const std::string& serverUri, 
            const std::string& username,
            const std::string& password,
            const std::string& clientId) 
        : connOpts_(), client_(serverUri, clientId)
    {
        if (!username.empty()) {
            connOpts_.set_user_name(username);
            connOpts_.set_password(password);
        }
        connOpts_.set_keep_alive_interval(20);
        connOpts_.set_clean_session(true);
    }

    void post(const std::string& topic, const std::string& value, bool retain = false) {
        ensureConnected();
        mqtt::message_ptr pubmsg = mqtt::make_message(topic, value);
        pubmsg->set_qos(1);
        if (!retain) {
            pubmsg->set_properties(mqtt::properties{mqtt::property(mqtt::property::MESSAGE_EXPIRY_INTERVAL, 300)});
        }
        pubmsg->set_retained(retain);
        if (!client_.publish(pubmsg)) {
            std::cerr << "Failed to post to topic: " << topic << std::endl;
        }
    }

private:
    mqtt::async_client client_;
    mqtt::connect_options connOpts_;

    void ensureConnected() {
        if (!client_.is_connected()) {
            client_.connect(connOpts_)->wait();
        }
    }

    static std::string envOrDefault(const char* env, const std::string& defaultValue) {
        const char* envValue = std::getenv(env);
        if (envValue != nullptr) {
            return envValue;
        }
        return defaultValue;
    }
};

