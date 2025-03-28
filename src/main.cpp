#include <iostream>
#include <time.h> 
#include "modbus.hpp"
#include "registerReader.hpp"
#include "registers.hpp"
#include "mqtt.hpp"
#include "hass.hpp"
#include "utils.hpp"


void getAndPost(Mqtt& mqtt) {
    static time_t lastMetadataUpdate = 0;
    const time_t secondsToForceMetadataUpdate = 60 * 60 * 2; // 2 hours

    Modbus modbus(envOrDefault("MODBUS_PORT", "/dev/ttyUSB0"), 19200, 'N', 8, 1, 1);
    RegisterReader registersReader(modbus, RegisterType::INPUT);
    RegisterReader holdingRegistersReader(modbus, RegisterType::HOLDING);
    std::unique_ptr<HassMqttDevice> inverter;
#ifdef EG418KPV
    RegistersEg418kpv r(registersReader, holdingRegistersReader);
    HassDevice device(r);
    inverter.reset(new HassInverterEg418kp(r, mqtt, device));
#elif defined(GRIDBOSS)
    RegistersGridBoss r(registersReader, holdingRegistersReader);
    HassDevice device(r);
    inverter.reset(new HassGridBoss(r, mqtt, device));
#else
#error Unknown device
#endif

    time_t now = time(0);
    if (now > secondsToForceMetadataUpdate + lastMetadataUpdate) {
        inverter->postDiscovery();
        lastMetadataUpdate = now;
    }

    inverter->postValues();
}


int main()
{
    try {
        Mqtt mqtt;
        while (true) {
            getAndPost(mqtt);
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
