#pragma once

#include "modbus.hpp"
#include <map>
#include <vector>

class ModbusDeviceManagerInterface {
public:
  virtual ~ModbusDeviceManagerInterface() = default;

  virtual uint16_t readInputRegister(int address) const = 0;
  virtual uint16_t readHoldRegister(int address) const = 0;
  virtual void writeRegister(int address, uint16_t value) const = 0;
};

class ModbusDeviceManager : public ModbusDeviceManagerInterface {
  static const int MODBUS_SEGMENT_SIZE = 40;
public:
  ModbusDeviceManager(const ModbusInterface& modbus) : modbus_(modbus) {
  }

  uint16_t readInputRegister(int address) const {
    if (inputCache_.find(address) == inputCache_.end()) {
      int startAddress = (address / MODBUS_SEGMENT_SIZE) * MODBUS_SEGMENT_SIZE;
      auto values = modbus_.readRegisters(RegisterType::INPUT, startAddress, MODBUS_SEGMENT_SIZE);
      for (int i = 0; i < MODBUS_SEGMENT_SIZE; ++i) {
        inputCache_[startAddress + i] = values[i];
      }
    }
    return inputCache_.at(address);
  }

  uint16_t readHoldRegister(int address) const {
    if (holdCache_.find(address) == holdCache_.end()) {
      int startAddress = (address / MODBUS_SEGMENT_SIZE) * MODBUS_SEGMENT_SIZE;
      auto values = modbus_.readRegisters(RegisterType::HOLDING, startAddress, MODBUS_SEGMENT_SIZE);
      for (int i = 0; i < MODBUS_SEGMENT_SIZE; ++i) {
        holdCache_[startAddress + i] = values[i];
      }
    }
    return holdCache_.at(address);
  }

  void writeRegister(int address, uint16_t value) const {
    std::vector<uint16_t> values(1, value);
    modbus_.writeRegisters(address, values);
    holdCache_[address] = value;
  }

protected:
  const ModbusInterface& modbus_;
  mutable std::map<int, uint16_t> inputCache_;
  mutable std::map<int, uint16_t> holdCache_;
};
