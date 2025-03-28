#include <iostream>
#include <time.h> 
#include "modbus.hpp"
#include "modbusDeviceManager.hpp"
#include "registers.hpp"
#include "mqtt.hpp"
#include "hass.hpp"
#include "utils.hpp"
#include "../gen/version.h"

void getAndPost(Mqtt& mqtt) {
    static time_t lastMetadataUpdate = 0;
    const time_t secondsToForceMetadataUpdate = 60 * 60 * 2; // 2 hours

    Modbus modbus(envOrDefault("MODBUS_PORT", "/dev/ttyUSB0"), 19200, 'N', 8, 1, 1);
    ModbusDeviceManager dm(modbus);
    std::unique_ptr<HassMqttDevice> hassMqttDevice;
#ifdef EG418KPV
    RegistersEg418kpv r(dm);
    HassDevice device(r);
    hassMqttDevice.reset(new HassInverterEg418kp(r, mqtt, device));
#elif defined(GRIDBOSS)
    RegistersGridBoss r(dm);
    HassDevice device(r);
    hassMqttDevice.reset(new HassGridBoss(r, mqtt, device));
#else
#error Unknown device
#endif

    time_t now = time(0);
    if (now > secondsToForceMetadataUpdate + lastMetadataUpdate) {
        hassMqttDevice->postDiscovery();
        lastMetadataUpdate = now;
    }

    hassMqttDevice->postValues();
}


int main()
{
    std::cerr << "EG4 Modbus Monitor " << APP_VERSION << "." << APP_GIT_HASH << std::endl;

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
