#pragma once

#include <sstream>
#include "registerReader.hpp"

class Registers
{
public:
    Registers(const RegisterReader &reader, const RegisterReader &holdingReader) 
        : reader_(reader), holdingReader_(holdingReader) 
    {
    }

    virtual ~Registers() = default;

    virtual std::string getSerial() const = 0;
    virtual std::string getFwVersion() const = 0;

protected:
    const RegisterReader &reader_;
    const RegisterReader &holdingReader_;

    uint16_t getRegister(int address) const { 
        return reader_.getRegister(address); 
    }

    std::string getRegisterString(const std::vector<int>& addresses) const {
        std::stringstream ss;
        for (auto addr : addresses) {
            int16_t value = getRegister(addr);
            ss << (char) (value & 0x00FF) << (char) (value >> 8);
        }
        return ss.str();
    }

    uint16_t getHoldRegister(int address) const { 
        return holdingReader_.getRegister(address); 
    }

    std::string getHoldRegisterString(const std::vector<int>& addresses) const {
        std::stringstream ss;
        for (auto addr : addresses) {
            int16_t value = getHoldRegister(addr);
            ss << (char) (value & 0x00FF) << (char) (value >> 8);
        }
        return ss.str();
    }

};

class RegistersEg418kpv: public Registers {
public:
    RegistersEg418kpv(const RegisterReader &reader, const RegisterReader &holdingReader) 
        : Registers(reader, holdingReader) 
    {
    }


#include "../gen/eg4-18kpv/register-accessors.inl"

};

class RegistersGridBoss: public Registers {
public:
    RegistersGridBoss(const RegisterReader &reader, const RegisterReader &holdingReader) 
        : Registers(reader, holdingReader) 
    {
    }


#include "../gen/eg4-gridboss/register-accessors.inl"

};
