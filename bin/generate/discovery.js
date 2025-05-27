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
    let component = reg.component ? reg.component : "sensor";
    let entity_category = reg.entity_category ? reg.entity_category : "";
    let device_class = reg.device_class ? reg.device_class :
        (unit == "V") ? "voltage" :
        (unit == "A") ? "current" :
        (unit == "W") ? "power" :
        (unit == "kWh") ? "energy" :
        (unit == "s") ? "duration" :
        (unit == "°C") ? "temperature" :
        "";
    let state_class = reg.state_class ? reg.state_class : "";
    let has_setter = reg.setter === undefined ? false : true;
    let enabled_by_default = reg.enabled_by_default === undefined ? true : reg.enabled_by_default;
    let attributes = reg.attributes ? JSON.stringify(reg.attributes) : "";

    console.log(`postDiscoveryEntry("${name}", "${icon}", "${component}", "${entity_category}", "${unit}", "${device_class}", "${state_class}", ${has_setter}, ${enabled_by_default}, R"(${attributes})" );`);
}
