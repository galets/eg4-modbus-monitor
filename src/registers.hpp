#pragma once

#include <sstream>
#include "registerReader.hpp"

class Registers
{
public:
    Registers(RegisterReader &reader, RegisterReader &holdingReader) 
        : reader_(reader), holdingReader_(holdingReader) 
    {
    }

#include "register-accessors.inl"

private:
    RegisterReader &reader_;
    RegisterReader &holdingReader_;

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
