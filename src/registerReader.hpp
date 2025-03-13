#pragma once

#include "modbus.hpp"
#include <map>
#include <vector>

class RegisterReader {
public:
  RegisterReader(ModbusReaderInterface &reader, RegisterType type) : reader_(reader), type_(type) {
    dropCaches();
  }

  void dropCaches() {
    for (int i = 0; i < CACHE_SIZE; ++i) {
      cache_[i] = std::vector<uint16_t>();
    }
  }

  uint16_t getRegister(int address) const {
    int cacheIndex = address / CACHE_SEGMENT_SIZE;
    int offset = address % CACHE_SEGMENT_SIZE;

    if (cacheIndex < 0 || cacheIndex >= CACHE_SIZE) {
      throw std::runtime_error("Invalid register address");
    }

    if (cache_[cacheIndex].empty()) {
      int startAddress = cacheIndex * CACHE_SEGMENT_SIZE;
      cache_[cacheIndex] = read(startAddress, CACHE_SEGMENT_SIZE);
    }

    if (offset < 0 || offset >= CACHE_SEGMENT_SIZE) {
      throw std::runtime_error("Invalid register address");
    }

    return cache_[cacheIndex][offset];
  }

protected:
  static const int CACHE_SEGMENT_SIZE = 40;
  static const int CACHE_SIZE = 10;

  ModbusReaderInterface &reader_;
  RegisterType type_;
  mutable std::map<int, std::vector<uint16_t>> cache_;

  std::vector<uint16_t> read(int start_address, int num_registers) const {
    if (num_registers <= 0) {
      throw std::runtime_error("Invalid number of registers to read");
    }

    if (start_address < 0) {
      throw std::runtime_error("Invalid start address");
    }    
    return reader_.readRegisters(type_, start_address, num_registers);
  }
};
