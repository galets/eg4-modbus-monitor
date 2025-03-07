const registers = JSON.parse(process.argv[2] || '[]')
for (let i=0; i<registers.length; i++) {
    let reg = registers[i];
    let type = reg.type == "string" ? "std::string" :
        reg.type == "int32" ? "int32_t" :
        reg.type == "int" ? "int16_t" : "double"
    let name = reg.item;
    let address = reg.address;
    let valueGetter = `getRegister(${address})`;
    if (reg.doubleSize) {
        valueGetter = `(getRegister(${address}) + getRegister(${address+1}) * 0x10000)`;
    }

    console.log("/**");
    console.log(`* @brief ${reg.description}`)
    console.log("*/");

    console.log(`${type} get${name}() const {`);
    if (reg.code) {
        for (const line of reg.code.split('\n')) {
            console.log(`    ${line}`);
        }
    } else if (reg.multiply) {        
        console.log(`    return ${valueGetter} * ${reg.multiply};`)
    } else {
        console.log(`    return ${valueGetter};`)
    }
    console.log("}");
    console.log("");
}
