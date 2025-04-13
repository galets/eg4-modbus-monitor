

all: 18kpv gridboss
18kpv: build/eg4-18kpv-reader
gridboss: build/gridboss-reader
tests: build/tests

.PHONY: all eg4-18kpv gridboss tests

gen/18kpv/%.inl: src/registers-18kpv.yaml bin/generate/%.js
	bin/run_generator.sh 18kpv $*

gen/gridboss/%.inl: src/registers-gridboss.yaml bin/generate/%.js
	bin/run_generator.sh gridboss $*

build/eg4-18kpv-reader: src/main.cpp src/modbusDeviceManager.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/18kpv/accessors.inl gen/18kpv/dump.inl gen/18kpv/json.inl \
		gen/18kpv/post.inl gen/18kpv/discovery.inl \
		gen/gridboss/accessors.inl gen/gridboss/dump.inl gen/gridboss/json.inl \
		gen/gridboss/post.inl gen/gridboss/discovery.inl
	mkdir -p build
	bin/update-version.sh
	g++ -DEG418KPV=1 -std=c++20 src/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -o build/eg4-18kpv-reader

build/gridboss-reader: src/main.cpp src/modbusDeviceManager.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/18kpv/accessors.inl gen/18kpv/dump.inl gen/18kpv/json.inl \
		gen/18kpv/post.inl gen/18kpv/discovery.inl \
		gen/gridboss/accessors.inl gen/gridboss/dump.inl gen/gridboss/json.inl \
		gen/gridboss/post.inl gen/gridboss/discovery.inl
	mkdir -p build
	bin/update-version.sh
	g++ -DGRIDBOSS=1 -std=c++20 src/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -o build/gridboss-reader

build/tests: tests/main.cpp tests/test_mqtt.cpp src/modbusDeviceManager.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/18kpv/accessors.inl gen/18kpv/dump.inl gen/18kpv/json.inl \
		gen/18kpv/post.inl gen/18kpv/discovery.inl \
		gen/gridboss/accessors.inl gen/gridboss/dump.inl gen/gridboss/json.inl \
		gen/gridboss/post.inl gen/gridboss/discovery.inl
	mkdir -p build
	bin/update-version.sh
	g++ -std=c++20 tests/main.cpp tests/test_mqtt.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -lgtest -lgtest_main -o build/tests

