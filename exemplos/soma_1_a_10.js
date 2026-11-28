"use strict";
function printf(formato, ...args) {
    let i = 0;
    const texto = formato.replace(/%(l?[dif]|c|%)/g, (_, esp) => {
        if (esp === "%") return "%";
        const v = args[i++];
        if (esp === "c") return String.fromCharCode(Number(v) & 0xFF);
        if (esp.endsWith("f")) {
            if (v === Infinity) return "inf";
            if (v === -Infinity) return "-inf";
            if (Object.is(v, -0)) return "-0.000000";
            return v.toFixed(6);
        }
        return String(v);
    });
    process.stdout.write(texto);
}
let i = 1;
let soma = 0;
while (i <= 10) {
    soma = ((soma + i) | 0);
    i = ((i + 1) | 0);
}
printf("Soma de 1 a 10 = %d\n", soma);
