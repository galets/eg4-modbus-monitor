#include <iostream>
#include <fstream>
#include <time.h> 
#include "../src/modbus.hpp"
#include "../src/registerReader.hpp"
#include "../src/registers.hpp"
#include "../src/mqtt.hpp"
#include "../src/hass.hpp"
#include "../src/utils.hpp"

int main()
{
    try {
        ModbusReader modbusReader(envOrDefault("MODBUS_PORT", "/dev/ttyUSB0"), 19200, 'N', 8, 1, 1);
        RegisterReader registersReader(modbusReader, RegisterType::INPUT);
        RegisterReader holdingRegistersReader(modbusReader, RegisterType::HOLDING);
        std::ofstream fr("regs.bin", std::ios::binary);
        std::ofstream hr("hold.bin", std::ios::binary);
        for (int i = 0; i < 200; ++i) {
            auto value = registersReader.getRegister(i);
            fr.write(reinterpret_cast<const char*>(&value), sizeof(value));
            value = holdingRegistersReader.getRegister(i);
            hr.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        fr.close();
        hr.close();
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
