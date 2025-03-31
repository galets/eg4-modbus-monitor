#include <iostream>
#include <time.h> 
#include "modbus.hpp"
#include "modbusDeviceManager.hpp"
#include "registers.hpp"
#include "mqtt.hpp"
#include "hass.hpp"
#include "utils.hpp"
#include "../gen/version.h"


int main()
{
    std::cerr << "EG4 Modbus Monitor " << APP_VERSION << "." << APP_GIT_HASH << std::endl;

    try {
        Mqtt mqtt;
        static time_t lastMetadataUpdate = 0;
        const time_t secondsToForceMetadataUpdate = 60 * 60 * 2; // 2 hours

        Modbus rawModbus(envOrDefault("MODBUS_PORT", "/dev/ttyUSB0"), 19200, 'N', 8, 1, 1);
        SerializedModbus modbus(rawModbus);
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

        while (true) {
            time_t now = time(0);
            if (now > secondsToForceMetadataUpdate + lastMetadataUpdate) {
                hassMqttDevice->postDiscovery();
                lastMetadataUpdate = now;
            }

            dm.dropCaches();
            hassMqttDevice->postValues();
            dm.dropCaches();

            modbus.wait(10000);
        }
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
