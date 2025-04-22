#include <cstdlib>
#include <cctype>
#include <iomanip>
#include <jsoncpp/json/json.h>
#include "utils.hpp"
#include "mqtt.hpp"
#include "registers.hpp"

// requires libjsoncpp-dev

class HassDevice {
public:
    std::string manufacturer;
    std::string name;
    std::string model;
    std::string serial;
    std::string sw_version;
    std::string identifier;

    HassDevice(const Registers& registers, const std::string& manufacturer, const std::string& model, const std::string& name) {
        this->manufacturer = manufacturer;
        this->model = model;
        this->name = name;
        serial = registers.getSerial();
        sw_version = registers.getFwVersion();
        identifier = to_lower(this->manufacturer + "_" + this->model + "_" + serial);
    }

    virtual ~HassDevice() = default;

    virtual Json::Value toJson() const {
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

class HassDevice18Kpv : public HassDevice {
    const RegistersEg418kpv& registers_;
public:
    HassDevice18Kpv(const RegistersEg418kpv& registers, const std::string& manufacturer, const std::string& model, const std::string& name) :
        HassDevice(registers, manufacturer, model, (name == "") ? manufacturer + " " + model + " Inverter" : name), registers_(registers) {
    }

    Json::Value toJson() const {
        Json::Value json = HassDevice::toJson();
#include "../gen/18kpv/json.inl"
        return json;
    }
};

class HassDeviceGridBoss : public HassDevice {
    const RegistersGridBoss& registers_;
public:
    HassDeviceGridBoss(const RegistersGridBoss& registers, const std::string& manufacturer, const std::string& model, const std::string& name) :
        HassDevice(registers, manufacturer, model, (name == "") ? manufacturer + " " + model : name), registers_(registers) {
    }

    Json::Value toJson() const {
        Json::Value json = HassDevice::toJson();
#include "../gen/gridboss/json.inl"
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
    virtual void postValues() const = 0;
    virtual void postDiscovery() const = 0;


protected:
    MqttInterface& mqtt_;
    const HassDevice& device_;
    mutable std::map<std::string, std::string> values_;

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
        auto valueStr = ss.str();
        if (values_.find(name) == values_.end() || values_[name] != valueStr) {
            std::cout << "posting to: " << topic << ": " << valueStr << std::endl;
        }
        values_[name] = valueStr;
        mqtt_.post(topic, valueStr);
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
        value["device"] = (name == "State") ? device_.toJson() : device_.toReducedJson();
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

class HassInverterEg418kp : public HassMqttDevice {
public:
    HassInverterEg418kp(const RegistersEg418kpv& registers, MqttInterface& mqtt, const HassDevice& device) :
        HassMqttDevice(mqtt, device),
        registers_(registers) {
    }

    void postValues() const {
#include "../gen/18kpv/post.inl"
    }

    void postDiscovery() const {
#include "../gen/18kpv/discovery.inl"
    }

private:
    const RegistersEg418kpv& registers_;
};

class HassGridBoss : public HassMqttDevice {
public:
    HassGridBoss(const RegistersGridBoss& registers, MqttInterface& mqtt, const HassDevice& device) :
        HassMqttDevice(mqtt, device),
        registers_(registers) {
    }

    void postValues() const {
#include "../gen/gridboss/post.inl"
    }

    void postDiscovery() const {
#include "../gen/gridboss/discovery.inl"
    }

private:
    const RegistersGridBoss& registers_;
};

