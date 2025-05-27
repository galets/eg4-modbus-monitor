#pragma once

#include <sstream>
#include "modbusDeviceManager.hpp"

class Registers
{
public:
    Registers(const ModbusDeviceManagerInterface& dm)
        : dm_(dm)
    {
    }

    virtual ~Registers() = default;

    virtual std::string getSerial() const = 0;
    virtual std::string getFwVersion() const = 0;

protected:
    const ModbusDeviceManagerInterface& dm_;

    uint16_t getRegister(int address) const {
        return dm_.readInputRegister(address);
    }

    std::string getRegisterString(const std::vector<int>& addresses) const {
        std::stringstream ss;
        for (auto addr : addresses) {
            int16_t value = getRegister(addr);
            ss << (char)(value & 0x00FF) << (char)(value >> 8);
        }
        return ss.str();
    }

    uint16_t getHoldRegister(int address) const {
        return dm_.readHoldRegister(address);
    }

    std::string getHoldRegisterString(const std::vector<int>& addresses) const {
        std::stringstream ss;
        for (auto addr : addresses) {
            int16_t value = getHoldRegister(addr);
            ss << (char)(value & 0x00FF) << (char)(value >> 8);
        }
        return ss.str();
    }

    void writeRegister(int address, uint16_t value) const {
        std::cout << "writing register (" << address << "): " << value << std::endl;
        dm_.writeRegister(address, value);
    }
};

class RegistersEg418kpv : public Registers {
public:
    RegistersEg418kpv(const ModbusDeviceManagerInterface& dm)
        : Registers(dm)
    {
    }


#include "../gen/18kpv/accessors.inl"

};

class RegistersGridBoss : public Registers {
public:
    RegistersGridBoss(const ModbusDeviceManagerInterface& dm)
        : Registers(dm)
    {
    }


#include "../gen/gridboss/accessors.inl"

};
