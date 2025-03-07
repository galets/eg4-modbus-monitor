

all: build/modbus-local-reader
.PHONY: all

src/register-accessors.inl: src/registers.yaml
	bin/run_generator.sh generate-accessors.js register-accessors.inl

src/register-dump.inl: src/registers.yaml
	bin/run_generator.sh generate-dump.js register-dump.inl

src/register-json.inl: src/registers.yaml
	bin/run_generator.sh generate-json.js register-json.inl

src/register-post.inl: src/registers.yaml
	bin/run_generator.sh generate-post.js register-post.inl

src/register-discovery.inl: src/registers.yaml
	bin/run_generator.sh generate-discovery.js register-discovery.inl

build/modbus-local-reader: src/main.cpp src/registerReader.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp src/register-accessors.inl \
		src/register-dump.inl src/register-json.inl src/register-post.inl src/register-discovery.inl
	mkdir -p build
	g++ -std=c++20 src/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -o build/modbus-local-reader
