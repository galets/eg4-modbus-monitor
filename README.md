# EG4 inverter monitor via modbus

This project implements a simple RS485 modbus monitor for EG4 inverters,
specifically tested on 18kPv

Collected data will be posted to MQTT server

## Build

```bash
# install dependencies
apt install git build-essential libpaho-mqttpp-dev libpaho-mqtt-dev libjsoncpp-dev libmodbus-dev libgtest-dev nodejs yq
# build
make 18kpv
make gridboss
```

## Hardware

This works with following hardware:

- EG4 18kpv
- EG4 GridBOSS (early alpha code, please report bugs). See some notes [here](doc/gridBoss.md)
- Raspberry Pi Zero 2W
- Following [RS485 USB dongle](https://www.amazon.com/dp/B081MB6PN2), or [even cheaper one](https://www.amazon.com/dp/B00NKAJGZM)
- Powered by [12 to 5V converter](https://www.amazon.com/dp/B09TFLZMC2)

## Configuration

Use environment variables to provide configuration. Example:

```bash
export MQTT_URI=tcp://localhost:1883
export MQTT_USERNAME=
export MQTT_PASSWORD=
export MQTT_CLIENT_ID=modbus-local-reader
export MODBUS_PORT=/dev/ttyUSB0
export DEVICE_MANUFACTURER=EG4
export DEVICE_MODEL=18kpv
export DEVICE_NAME="EG4 18kpv Inverter"
build/eg4-18kpv-reader

export DEVICE_MODEL=gridboss
export DEVICE_NAME="EG4 GridBOSS"
build/gridboss-reader
```
