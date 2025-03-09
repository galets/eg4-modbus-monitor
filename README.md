# EG4 inverter monitor via modbus

This project implements a simple RS485 modbus monitor for EG4 inverters,
specifically tested on 18kPv

Collected data will be posted to MQTT server

## Build requirements

```bash
apt install git build-essential libpaho-mqttpp-dev libpaho-mqtt-dev libjsoncpp-dev libmodbus-dev nodejs yq
```

## Hardware

This works with following hardware:

- EG4 18kpv
- Raspberry Pi Zero 2W
- Following [RS485 USB dongle](https://www.amazon.com/dp/B081MB6PN2)
- Powered by [12 to 5V converter](https://www.amazon.com/dp/B09TFLZMC2)

## Configuration

Use environment variables to provide configuration. Example:

```bash
export MQTT_URI=tcp://localhost:1883
export MQTT_USERNAME=
export MQTT_PASSWORD=
export MQTT_CLIENT_ID=modbus-local-reader
export MODBUS_PORT=/dev/ttyUSB0
build/modbus-local-reader
```
