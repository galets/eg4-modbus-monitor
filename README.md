# EG4 inverter monitor via modbus

This project implements a simple RS485 modbus monitor for EG4 inverters,
specifically tested on 18kPv

Collected data will be posted to MQTT server

**NOTE**: This project is no longer under active development, as it is superceded by
[eg4 dongle](https://github.com/galets/eg4-dongle) project providing much better integration with
Home Assistant without a need to support additional hardware

## Build

```bash
# install dependencies
apt install git build-essential libpaho-mqttpp-dev libpaho-mqtt-dev libjsoncpp-dev libmodbus-dev libgtest-dev nodejs yq
# build
make clean
make
```

## Hardware

This works with following hardware:

- EG4 18kpv
- EG4 GridBOSS (early alpha code, please report bugs). See some notes [here](doc/gridBoss.md)
- Raspberry Pi Zero 2W
- Following [RS485 USB dongle](https://www.amazon.com/dp/B081MB6PN2), or [even cheaper one](https://www.amazon.com/dp/B00NKAJGZM)
- Powered by [12 to 5V converter](https://www.amazon.com/dp/B09TFLZMC2)

## Command line parameters

Use following command line parameters to run the program:

```text
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
```

If MQTT URI is not specified, the program will print all available parameters from device
in JSON format and exit.

If MQTT_URI is specified, the program will use MQTT server to post values every 10 seconds.
Also, an [MQTT auto-discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
entries would be posted, so that Home Assistant can automatically find and configure device

### Configuration using environment values

Environment variables can also be used to provide configuration. Example:

```bash
export MQTT_URI=tcp://localhost:1883
# empty username indicated anonymous access will be used
export MQTT_USERNAME=
export MQTT_PASSWORD=
export MQTT_CLIENT_ID=modbus-local-reader
export MODBUS_PORT=/dev/ttyUSB0
export DEVICE_MANUFACTURER=EG4
export DEVICE_MODEL=18kpv
export DEVICE_NAME="EG4 18kpv Inverter"
build/eg4-modbus-reader

export DEVICE_MODEL=gridboss
export DEVICE_NAME="EG4 GridBOSS"
build/eg4-modbus-reader 
```

## Contact

This project will be maintained on github at https://github.com/galets/eg4-modbus-monitor

Discussion group will be available on Telegram at https://t.me/eg4modbus
