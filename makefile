

all: build/eg4-18kpv-reader
18kpv: build/eg4-18kpv-reader
gridboss: build/gridboss-reader
tests: build/tests

.PHONY: all eg4-18kpv gridboss tests

gen/eg4-18kpv/register-accessors.inl: src/registers-eg4-18kpv.yaml
	bin/run_generator.sh eg4-18kpv generate-accessors.js register-accessors.inl

gen/eg4-18kpv/register-dump.inl: src/registers-eg4-18kpv.yaml
	bin/run_generator.sh eg4-18kpv generate-dump.js register-dump.inl

gen/eg4-18kpv/register-json.inl: src/registers-eg4-18kpv.yaml
	bin/run_generator.sh eg4-18kpv generate-json.js register-json.inl

gen/eg4-18kpv/register-post.inl: src/registers-eg4-18kpv.yaml
	bin/run_generator.sh eg4-18kpv generate-post.js register-post.inl

gen/eg4-18kpv/register-discovery.inl: src/registers-eg4-18kpv.yaml
	bin/run_generator.sh eg4-18kpv generate-discovery.js register-discovery.inl

gen/eg4-gridboss/register-accessors.inl: src/registers-eg4-gridboss.yaml
	bin/run_generator.sh eg4-gridboss generate-accessors.js register-accessors.inl

gen/eg4-gridboss/register-dump.inl: src/registers-eg4-gridboss.yaml
	bin/run_generator.sh eg4-gridboss generate-dump.js register-dump.inl

gen/eg4-gridboss/register-json.inl: src/registers-eg4-gridboss.yaml
	bin/run_generator.sh eg4-gridboss generate-json.js register-json.inl

gen/eg4-gridboss/register-post.inl: src/registers-eg4-gridboss.yaml
	bin/run_generator.sh eg4-gridboss generate-post.js register-post.inl

gen/eg4-gridboss/register-discovery.inl: src/registers-eg4-gridboss.yaml
	bin/run_generator.sh eg4-gridboss generate-discovery.js register-discovery.inl

build/eg4-18kpv-reader: src/main.cpp src/registerReader.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/eg4-18kpv/register-accessors.inl gen/eg4-18kpv/register-dump.inl gen/eg4-18kpv/register-json.inl \
		gen/eg4-18kpv/register-post.inl gen/eg4-18kpv/register-discovery.inl \
		gen/eg4-gridboss/register-accessors.inl gen/eg4-gridboss/register-dump.inl gen/eg4-gridboss/register-json.inl \
		gen/eg4-gridboss/register-post.inl gen/eg4-gridboss/register-discovery.inl
	mkdir -p build
	g++ -DEG418KPV=1 -std=c++20 src/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -o build/eg4-18kpv-reader

build/gridboss-reader: src/main.cpp src/registerReader.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/eg4-18kpv/register-accessors.inl gen/eg4-18kpv/register-dump.inl gen/eg4-18kpv/register-json.inl \
		gen/eg4-18kpv/register-post.inl gen/eg4-18kpv/register-discovery.inl \
		gen/eg4-gridboss/register-accessors.inl gen/eg4-gridboss/register-dump.inl gen/eg4-gridboss/register-json.inl \
		gen/eg4-gridboss/register-post.inl gen/eg4-gridboss/register-discovery.inl
	mkdir -p build
	g++ -DGRIDBOSS=1 -std=c++20 src/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -o build/gridboss-reader

build/tests: tests/main.cpp src/registerReader.hpp src/registers.hpp \
		src/modbus.hpp src/mqtt.hpp src/hass.hpp \
		gen/eg4-18kpv/register-accessors.inl gen/eg4-18kpv/register-dump.inl gen/eg4-18kpv/register-json.inl \
		gen/eg4-18kpv/register-post.inl gen/eg4-18kpv/register-discovery.inl \
		gen/eg4-gridboss/register-accessors.inl gen/eg4-gridboss/register-dump.inl gen/eg4-gridboss/register-json.inl \
		gen/eg4-gridboss/register-post.inl gen/eg4-gridboss/register-discovery.inl
	mkdir -p build
	g++ -std=c++20 tests/main.cpp -lmodbus -ljsoncpp -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -lgtest -lgtest_main -o build/tests

