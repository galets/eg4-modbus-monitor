const registers = JSON.parse(process.argv[2] || '[]')
for (let i=0; i<registers.length; i++) {
    let reg = registers[i];
    let name = reg.item;
    let unit = reg.unit ? reg.unit : "";
    let icon = reg.icon ? reg.icon : 
        (unit == "V") ? "mdi:lightning-bolt" :
        (unit == "A") ? "mdi:current-ac" :
        (unit == "W") ? "mdi:power" :
        (unit == "kWh") ? "mdi:meter-electric" :
        (unit == "s") ? "mdi:timer" :
        (unit == "°C") ? "mdi:thermometer" :
        "";
    let entity_category = reg.entity_category ? reg.entity_category : "";
    let device_class = reg.device_class ? reg.device_class :
        (unit == "V") ? "voltage" :
        (unit == "A") ? "current" :
        (unit == "W") ? "power" :
        (unit == "kWh") ? "energy" :
        (unit == "s") ? "duration" :
        (unit == "°C") ? "temperature" :
        "";
    let state_class = reg.state_class ? reg.state_class : ""

    console.log(`postDiscoveryEntry("${name}", "${icon}", "${entity_category}", "${unit}", "${device_class}", "${state_class}");`);
}
