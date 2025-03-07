#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <iomanip>
#include <jsoncpp/json/json.h>
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
        manufacturer = envOrDefault("INVERTER_MANUFACTURER", "EG4");
        model = envOrDefault("INVERTER_MODEL", "18kPv");
        name = envOrDefault("INVERTER_NAME", manufacturer + " " + model + " Inverter");
        serial = registers.getSerial();
        sw_version = registers.getFwVersion();
        identifier = manufacturer + "_" + model + "_" + serial;
        std::transform(identifier.begin(), identifier.end(), identifier.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    static std::string envOrDefault(const char* env, const std::string& defaultValue) {
        const char* envValue = std::getenv(env);
        if (envValue != nullptr) {
            return envValue;
        }
        return defaultValue;
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

class HassInverter {
public:
    HassInverter(const Registers& registers, Mqtt& mqtt, const HassDevice& device) :
        registers_(registers),
        mqtt_(mqtt),
        device_(device) {
    }

    Json::Value toJson() const {
        Json::Value json;
#include "register-json.inl"
        return json;
    }

    void postValues() const {
#include "register-post.inl"
    }

    void postDiscovery() const {
#include "register-discovery.inl"
    }

private:
    const Registers& registers_;
    Mqtt& mqtt_;
    const HassDevice& device_;

    std::string getTopic(const std::string& name) const {
        return "modbus/" + device_.identifier + "/" + name;
    }

    template <typename T>
    void postValue(const std::string& name, T value) const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << value;
        auto topic = getTopic(name);
        std::cout << "posting to: " << topic << ": " << ss.str() << std::endl;
        mqtt_.post(topic, ss.str());
    }

    void postDiscoveryEntry(const std::string& name, const std::string& icon, const std::string& entity_category, 
            const std::string& unit_of_measurement, const std::string& device_class, const std::string& state_class) const {
        Json::Value value;
        value["component"] = "sensor";
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

        auto topic = "homeassistant/sensor/" + device_.identifier + "/" + name + "/config";
        //std::cout << "posting metadata to: " << topic << std::endl;
        mqtt_.post(topic, value.toStyledString(), true);
    }
};
