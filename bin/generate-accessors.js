const registers = JSON.parse(process.argv[2] || '[]')
let prevAddress = 0;
for (let i=0; i<registers.length; i++) {
    let reg = registers[i];
    let type = reg.type == "string" ? "std::string" :
        reg.type == "int32" ? "int32_t" :
        reg.type == "int" ? "int16_t" : "double"
    let name = reg.item;
    let address = reg.address ? reg.address : (prevAddress + 1);
    prevAddress = address;

    let unit = reg.unit ? reg.unit : "";
    let multiply = reg.multiply ? reg.multiply : 
        (unit == "V") ? 0.1 :
        (unit == "A") ? 0.01 :
        (unit == "W") ? 1 :
        (unit == "Wh") ? 0.1 :
        (unit == "kWh") ? 0.1 :
        (unit == "Hz") ? 0.01 :
        1;

    let signed = reg.signed !== undefined ? reg.signed :
        (unit == "W") ? true :
        false;

    let valueGetter = signed ? `static_cast<int16_t>(getRegister(${address}))` : `getRegister(${address})`;
    if (reg.doubleSize) {
        valueGetter = `(getRegister(${address}) + getRegister(${address+1}) * 0x10000)`;
        if (signed) {
            valueGetter = `static_cast<int32_t>(${valueGetter})`;
        }
        prevAddress++;
    }

    console.log("/**");
    console.log(`* @brief ${reg.description}`)
    console.log("*/");

    console.log(`${type} get${name}() const {`);
    if (reg.code) {
        for (const line of reg.code.split('\n')) {
            console.log(`    ${line}`);
        }
    } else if (multiply) {        
        console.log(`    return ${valueGetter} * ${multiply};`)
    } else {
        console.log(`    return ${valueGetter};`)
    }
    console.log("}");
    console.log("");

    if (reg.setter) {
        console.log(`void set${name}(const std::string& value) const {`);
        for (const line of reg.setter.split('\n')) {
            console.log(`    ${line}`);
        }
        console.log("}");
        console.log("");
    }
}
