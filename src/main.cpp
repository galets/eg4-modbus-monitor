#include "../gen/version.h"
#include "hass.hpp"
#include "modbus.hpp"
#include "modbusDeviceManager.hpp"
#include "mqtt.hpp"
#include "registers.hpp"
#include "utils.hpp"
#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <string>
#include <time.h> 
#include <vector>

struct Config {
    std::string mqtt_uri;
    std::string mqtt_username;
    std::string mqtt_password;
    std::string mqtt_client_id;
    std::string modbus_port;
    std::string device_manufacturer;
    std::string device_model;
    std::string device_name;
    bool header = true;
    bool verbose = false;
};

std::string get_env_var(const char* name, const std::string& default_val = "") {
    const char* value = std::getenv(name);
    return (value && value[0] != '\0') ? std::string(value) : default_val;
}


// Function to print usage instructions using std::cerr and an R-string
void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " [options]\n\n";

    std::cerr << R"(Options:
  --device-model <model>    REQUIRED: Device Model (e.g., 18kpv or gridboss)
                            (Default: $DEVICE_MODEL)
  --device-manufacturer <m> Device Manufacturer
                            (Default: $DEVICE_MANUFACTURER or 'EG4')
  --device-name <name>      Device Name (friendly name)
                            (Default: $DEVICE_NAME or '')
  --modbus-port <device>    Serial port for Modbus RTU
                            (Default: $MODBUS_PORT or '/dev/ttyUSB0')
  --mqtt-uri <uri>          MQTT broker URI, for example: tcp://localhost:1883.
                            Default: $MQTT_URI
                            if not specified, no MQTT interaction is performed
  --mqtt-username <user>    MQTT username
                            (Default: $MQTT_USERNAME)
  --mqtt-password <pass>    MQTT password
                            (Default: $MQTT_PASSWORD)
  --mqtt-client-id <id>     MQTT client ID, use different IDs for different devices
                            (Default: $MQTT_CLIENT_ID or 'modbus-reader')
  -v, --verbose             Show more diagnostic information
  -n, --no-header           Suppress header
  -h, --help                Show this help message and exit

Environment variables can be used to set default values.
)";
}

Config parse_command_line(int argc, char* argv[]) {
    Config config;

    config.mqtt_uri = get_env_var("MQTT_URI");
    config.mqtt_username = get_env_var("MQTT_USERNAME");
    config.mqtt_password = get_env_var("MQTT_PASSWORD");
    config.mqtt_client_id = get_env_var("MQTT_CLIENT_ID", "eg4-modbus-reader");
    config.modbus_port = get_env_var("MODBUS_PORT", "/dev/ttyUSB0");
    config.device_manufacturer = get_env_var("DEVICE_MANUFACTURER", "EG4");
    config.device_model = get_env_var("DEVICE_MODEL");
    config.device_name = get_env_var("DEVICE_NAME");

    enum {
        OPT_MQTT_URI = 1000,
        OPT_MQTT_USERNAME,
        OPT_MQTT_PASSWORD,
        OPT_MQTT_CLIENT_ID,
        OPT_MODBUS_PORT,
        OPT_DEVICE_MANUFACTURER,
        OPT_DEVICE_MODEL,
        OPT_DEVICE_NAME,
    };

    static struct option long_options[] = {
        {"mqtt-uri",            required_argument, 0, OPT_MQTT_URI },
        {"mqtt-username",       required_argument, 0, OPT_MQTT_USERNAME },
        {"mqtt-password",       required_argument, 0, OPT_MQTT_PASSWORD },
        {"mqtt-client-id",      required_argument, 0, OPT_MQTT_CLIENT_ID },
        {"modbus-port",         required_argument, 0, OPT_MODBUS_PORT },
        {"device-manufacturer", required_argument, 0, OPT_DEVICE_MANUFACTURER },
        {"device-model",        required_argument, 0, OPT_DEVICE_MODEL },
        {"device-name",         required_argument, 0, OPT_DEVICE_NAME },
        {"verbose",             no_argument,       0, 'v' },
        {"no-header",           no_argument,       0, 'n' },
        {"help",                no_argument,       0, 'h' },
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    const char* short_opts = "h";

    opterr = 0;
    while ((opt = getopt_long(argc, argv, short_opts, long_options, &option_index)) != -1) {
        switch (opt) {
        case OPT_MQTT_URI:            config.mqtt_uri = optarg; break;
        case OPT_MQTT_USERNAME:       config.mqtt_username = optarg; break;
        case OPT_MQTT_PASSWORD:       config.mqtt_password = optarg; break;
        case OPT_MQTT_CLIENT_ID:      config.mqtt_client_id = optarg; break;
        case OPT_MODBUS_PORT:         config.modbus_port = optarg; break;
        case OPT_DEVICE_MANUFACTURER: config.device_manufacturer = optarg; break;
        case OPT_DEVICE_MODEL:        config.device_model = optarg; break;
        case OPT_DEVICE_NAME:         config.device_name = optarg; break;
        case 'h':
            print_usage(argv[0]);
            exit(EXIT_SUCCESS);
        case 'v':
            config.verbose = true;
            break;
        case 'n':
            config.header = false;
            break;
        case '?':
            if (optopt == 0) {
                std::cerr << "Error: Unknown option or missing argument for '" << argv[optind - 1] << "'.\n";
            }
            else {
                std::cerr << "Error: Unknown option '-" << (char)optopt << "' or missing argument.\n";
            }
            print_usage(argv[0]);
            exit(EXIT_FAILURE);
        default: // Should not happen
            std::cerr << "Error: Unexpected getopt_long return value: " << opt << "\n";
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc) {
        std::cerr << "Error: Unexpected non-option arguments found: ";
        while (optind < argc) {
            std::cerr << argv[optind++] << " ";
        }
        std::cerr << "\n";
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return config;
}


int main(int argc, char* argv[])
{
    try {
        Config config = parse_command_line(argc, argv);
        if (config.header) {
            std::cerr << "EG4 Modbus Monitor " << APP_VERSION << "." << APP_GIT_HASH << std::endl;
        }

        if (config.verbose) {
            std::cerr << "Configuration:\n";
            std::cerr << "  MQTT URI:           " << config.mqtt_uri << "\n";
            std::cerr << "  MQTT Username:      " << config.mqtt_username << "\n";
            std::cerr << "  MQTT Password:      " << "********" << "\n";
            std::cerr << "  MQTT Client ID:     " << config.mqtt_client_id << "\n";
            std::cerr << "  Modbus Port:        " << config.modbus_port << "\n";
            std::cerr << "  Device Manufacturer:" << config.device_manufacturer << "\n";
            std::cerr << "  Device Model:       " << config.device_model << "\n";
            std::cerr << "  Device Name:        " << config.device_name << "\n";
        }

        Modbus rawModbus(config.modbus_port, 19200, 'N', 8, 1, 1);
        SerializedModbus modbus(rawModbus);
        ModbusDeviceManager dm(modbus);
        std::unique_ptr<Registers> registers;
        std::unique_ptr<HassDevice> device;
        std::string device_model_lower = to_lower(config.device_model);
        if (device_model_lower == "18kpv") {
            auto r = new RegistersEg418kpv(dm);
            registers.reset(r);
            device.reset(new HassDevice18Kpv(*r, config.device_manufacturer, config.device_model, config.device_name));
        }
        else if (device_model_lower == "gridboss") {
            auto r = new RegistersGridBoss(dm);
            registers.reset(r);
            device.reset(new HassDeviceGridBoss(*r, config.device_manufacturer, config.device_model, config.device_name));
        }
        else {
            throw std::runtime_error("Unknown device model: " + config.device_model);
        }

        if (config.mqtt_uri == "") {
            std::cout << device->toJson() << std::endl;
            return 0;
        }
        else {
            Mqtt mqtt(config.mqtt_uri, config.mqtt_username, config.mqtt_password, config.mqtt_client_id);
            std::unique_ptr<HassMqttDevice> hassMqttDevice;
            if (device_model_lower == "18kpv") {
                hassMqttDevice.reset(new HassInverterEg418kp(dynamic_cast<RegistersEg418kpv&>(*registers), mqtt, *device));
            }
            else if (device_model_lower == "gridboss") {
                hassMqttDevice.reset(new HassGridBoss(dynamic_cast<RegistersGridBoss&>(*registers), mqtt, *device));
            }
            else {
                throw std::runtime_error("Unknown device model: " + config.device_model);
            }

            static time_t lastMetadataUpdate = 0;
            const time_t secondsToForceMetadataUpdate = 60 * 60 * 2; // 2 hours

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
