#!/bin/bash

cd "$(dirname "$0")/.." || exit 1

# Teste 1: palavras reservadas e identificadores
gcc -Isrc tests/test_scanner.c src/lex.yy.c -o tests/test_scanner || exit 1

resultado=$(./tests/test_scanner tests/entradas_scanner/controle_condicional.txt)

esperado="IF if
ELSE else
ID ifelse_var"

if [ "$resultado" = "$esperado" ]; then
    echo "[OK] Palavras reservadas e identificadores"
else
    echo "[FALHOU] Palavras reservadas e identificadores"
    echo "Resultado obtido:"
    echo "$resultado"
    rm -f tests/test_scanner
    exit 1
fi

rm -f tests/test_scanner

# Teste 2: diretiva include com final de linha LF
arquivo=$(mktemp)
trap 'rm -f "$arquivo"' EXIT

printf '#include <stdio.h>\nint x = 10;\n' > "$arquivo"

if ./src/parser "$arquivo" > /dev/null 2>&1; then
    echo "[OK] Include com final de linha LF"
else
    echo "[FALHOU] Include com final de linha LF"
    exit 1
fi

# Teste 3: diretiva include com final de linha CRLF
printf '#include <stdio.h>\r\nint x = 10;\r\n' > "$arquivo"

if ./src/parser "$arquivo" > /dev/null 2>&1; then
    echo "[OK] Include com final de linha CRLF"
else
    echo "[FALHOU] Include com final de linha CRLF"
    exit 1
fi

echo "Teste do scanner: OK"
