const registers = JSON.parse(process.argv[2] || '[]')
for (let i=0; i<registers.length; i++) {
    let reg = registers[i];
    let name = reg.item;
    let unit = reg.unit ? ` ${reg.unit}`: '';
    console.log(`json["${name}"] = registers_.get${name}();`);
}
