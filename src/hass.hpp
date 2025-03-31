#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <iomanip>
#include <jsoncpp/json/json.h>
#include "utils.hpp"
#include "mqtt.hpp"
#include "registers.hpp"

// requires libjsoncpp-dev

struct HassDevice {
    std::string manufacturer;
    std::string name;
    std::string model;
    std::string serial;
    std::string sw_version;
    std::string identifier;

    HassDevice(const Registers& registers) {
        manufacturer = envOrDefault("DEVICE_MANUFACTURER", "EG4");
        model = envOrError("DEVICE_MODEL");
        name = envOrDefault("DEVICE_NAME", manufacturer + " " + model + " Inverter");
        serial = registers.getSerial();
        sw_version = registers.getFwVersion();
        identifier = manufacturer + "_" + model + "_" + serial;
        std::transform(identifier.begin(), identifier.end(), identifier.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    Json::Value toJson() const {
        Json::Value json;
        json["manufacturer"] = manufacturer;
        json["name"] = name;
        json["model"] = model;
        json["serial_number"] = serial;
        json["sw_version"] = sw_version;
        Json::Value identifiersJson(Json::arrayValue);
        identifiersJson.append(identifier);
        json["identifiers"] = identifiersJson;
        return json;
    }

    Json::Value toReducedJson() const {
        Json::Value json;
        Json::Value identifiersJson(Json::arrayValue);
        identifiersJson.append(identifier);
        json["identifiers"] = identifiersJson;
        return json;
    }
};

class HassMqttDevice {
public:
    HassMqttDevice(MqttInterface& mqtt, const HassDevice& device) :
        mqtt_(mqtt),
        device_(device) {
    }

    virtual ~HassMqttDevice() = default;
    virtual Json::Value toJson() const = 0;
    virtual void postValues() const = 0;
    virtual void postDiscovery() const = 0;


protected:
    MqttInterface& mqtt_;
    const HassDevice& device_;

    std::string getTopic(const std::string& name) const {
        return "modbus/" + device_.identifier + "/" + name;
    }

    std::string getCmdTopic(const std::string& name) const {
        return "modbus/cmd/" + device_.identifier + "/" + name;
    }

    template <typename T>
    void postValue(const std::string& name, T value) const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << value;
        auto topic = getTopic(name);
        std::cout << "posting to: " << topic << ": " << ss.str() << std::endl;
        mqtt_.post(topic, ss.str());
    }

    void subscribe(const std::string& name, std::function<void(const std::string&)> callback) const {
        auto topic = getCmdTopic(name);
        mqtt_.listen(topic, callback);
    }

    void postDiscoveryEntry(const std::string& name, const std::string& icon, const std::string& component, const std::string& entity_category, 
            const std::string& unit_of_measurement, const std::string& device_class, const std::string& state_class,
            bool has_setter, bool enabled_by_default) const {
        Json::Value value;
        value["component"] = component;
        value["device"] = (name == "State")? device_.toJson(): device_.toReducedJson();
        if (!device_class.empty()) {
            value["device_class"] = device_class;
        }
        if (!entity_category.empty()) {
            value["entity_category"] = entity_category;
        }
        if (!icon.empty()) {
            value["icon"] = icon;
        }
        value["name"] = name;
        value["unique_id"] = device_.identifier + "_" + name;
        if (!unit_of_measurement.empty()) {
            value["unit_of_measurement"] = unit_of_measurement;
        }
        if (!state_class.empty()) {
            value["state_class"] = state_class;
        }
        value["state_topic"] = getTopic(name);
        if (has_setter) {
            value["command_topic"] = getCmdTopic(name);
        }
        if (!enabled_by_default) {
            value["enabled_by_default"] = false;
        }

        auto topic = "homeassistant/" + component + "/" + device_.identifier + "/" + name + "/config";
        //std::cout << "posting metadata to: " << topic << std::endl;
        mqtt_.post(topic, value.toStyledString(), true);
    }
};

class HassInverterEg418kp: public HassMqttDevice {
public:
    HassInverterEg418kp(const RegistersEg418kpv& registers, MqttInterface& mqtt, const HassDevice& device) :
        HassMqttDevice(mqtt, device),
        registers_(registers) {
    }

    Json::Value toJson() const {
        Json::Value json;
#include "../gen/eg4-18kpv/register-json.inl"
        return json;
    }

    void postValues() const {
#include "../gen/eg4-18kpv/register-post.inl"
    }

    void postDiscovery() const {
#include "../gen/eg4-18kpv/register-discovery.inl"
    }

private:
    const RegistersEg418kpv& registers_;
};

class HassGridBoss: public HassMqttDevice {
    public:
        HassGridBoss(const RegistersGridBoss& registers, MqttInterface& mqtt, const HassDevice& device) :
            HassMqttDevice(mqtt, device),
            registers_(registers) {
        }
    
        Json::Value toJson() const {
            Json::Value json;
    #include "../gen/eg4-gridboss/register-json.inl"
            return json;
        }
    
        void postValues() const {
    #include "../gen/eg4-gridboss/register-post.inl"
        }
    
        void postDiscovery() const {
    #include "../gen/eg4-gridboss/register-discovery.inl"
        }
    
    private:
        const RegistersGridBoss& registers_;
    };
    