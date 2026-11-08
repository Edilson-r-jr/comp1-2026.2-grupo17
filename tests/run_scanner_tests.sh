#!/bin/bash

gcc -Isrc tests/test_scanner.c src/lex.yy.c -o tests/test_scanner

resultado=$(./tests/test_scanner tests/entradas_scanner/controle_condicional.txt)

esperado="token=263 texto=if
token=264 texto=else
token=262 texto=ifelse_var"

if [ "$resultado" = "$esperado" ]; then
    echo "Teste do scanner: OK"
else
    echo "Teste do scanner: FALHOU"
    echo "Resultado obtido:"
    echo "$resultado"
    rm -f tests/test_scanner
    exit 1
fi

rm -f tests/test_scanner
