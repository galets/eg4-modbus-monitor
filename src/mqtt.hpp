#pragma once

#include <string>
#include <iostream>
#include <mqtt/async_client.h>
#include "utils.hpp"

// requires libpaho-mqttpp-dev and libpaho-mqtt-dev

class MqttInterface
{
public:
    virtual ~MqttInterface() = default;
    virtual void post(const std::string& topic, const std::string& value, bool retain = false) = 0;
    virtual void listen(const std::string& topic, std::function<void(const std::string&)> callback) = 0;
};

class Mqtt : public MqttInterface
{

public:
    Mqtt(const std::string& serverUri,
        const std::string& username,
        const std::string& password,
        const std::string& clientId)
        : connOpts_(), client_(serverUri, clientId)
    {
        if (!username.empty())
        {
            connOpts_.set_user_name(username);
            connOpts_.set_password(password);
        }
        connOpts_.set_keep_alive_interval(20);
        connOpts_.set_clean_session(true);
    }

    void post(const std::string& topic, const std::string& value, bool retain = false)
    {
        ensureConnected();
        mqtt::message_ptr pubmsg = mqtt::make_message(topic, value);
        pubmsg->set_qos(1);
        if (!retain)
        {
            pubmsg->set_properties(mqtt::properties{ mqtt::property(mqtt::property::MESSAGE_EXPIRY_INTERVAL, 300) });
        }
        pubmsg->set_retained(retain);
        if (!client_.publish(pubmsg))
        {
            std::cerr << "Failed to post to topic: " << topic << std::endl;
        }
    }

    void listen(const std::string& topic, std::function<void(const std::string&)> callback)
    {
        ensureConnected();
        auto message_cb = [callback](mqtt::const_message_ptr msg)
            {
                if (msg)
                {
                    std::string payload = msg->get_payload_str();
                    callback(payload);
                }
            };

        client_.set_message_callback(message_cb);

        class connection_listener : public virtual mqtt::iaction_listener
        {
        public:
            connection_listener(mqtt::async_client& client,
                const std::string& topic,
                int qos) : client_(client), topic_(topic), qos_(qos) {
            }

            void on_success(const mqtt::token& tok) override
            {
                client_.subscribe(topic_, qos_, nullptr, subscription_listener_);
            }

            void on_failure(const mqtt::token& tok) override
            {
                std::cerr << "Connection failed (in listen)" << std::endl;
            }

        private:
            mqtt::async_client& client_;
            std::string topic_;
            int qos_;

            class subscription_listener : public virtual mqtt::iaction_listener
            {
            public:
                void on_success(const mqtt::token& tok) override
                {
                }

                void on_failure(const mqtt::token& tok) override
                {
                    std::cerr << "Subscription failed" << std::endl;
                }
            } subscription_listener_;
        };

        client_.subscribe(topic, 1); // QoS level 1
    }

private:
    mqtt::async_client client_;
    mqtt::connect_options connOpts_;

    void ensureConnected()
    {
        if (!client_.is_connected())
        {
            client_.connect(connOpts_)->wait();
            if (!client_.is_connected())
            {
                throw std::runtime_error("Failed to connect to MQTT broker");
            }
        }
    }
};
