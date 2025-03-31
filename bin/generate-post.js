const registers = JSON.parse(process.argv[2] || '[]')
for (let i=0; i<registers.length; i++) {
    let reg = registers[i];
    let name = reg.item;
    let unit = reg.unit ? ` ${reg.unit}`: '';
    console.log(`postValue("${name}", registers_.get${name}());`);
    if (reg.setter) {
        console.log(`subscribe("${name}", [this](const std::string& value) { registers_.set${name}(value); });`);
    }
}
