#include <iostream>
#include "modbus.hpp"
#include "registerReader.hpp"
#include "registers.hpp"
#include "mqtt.hpp"
#include "hass.hpp"

int main()
{
    try {
        ModbusReader modbusReader("/dev/ttyUSB0", 19200, 'N', 8, 1, 1);
        RegisterReader registersReader(modbusReader, RegisterType::INPUT);
        RegisterReader holdingRegistersReader(modbusReader, RegisterType::HOLDING);
        Registers r(registersReader, holdingRegistersReader);
        HassDevice device(r);
        Mqtt mqtt;
        HassInverter inverter(r, mqtt, device);
    
        inverter.postDiscovery();

        while (true) {
            inverter.postValues();
            registersReader.dropCaches();
            holdingRegistersReader.dropCaches();
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
