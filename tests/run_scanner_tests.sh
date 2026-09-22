#!/bin/bash

gcc -Isrc tests/test_scanner.c src/lex.yy.c -o tests/test_scanner

resultado=$(./tests/test_scanner tests/entradas_scanner/controle_condicional.txt)

esperado="token=267 texto=if
token=268 texto=else
token=266 texto=ifelse_var
token=265 texto="

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
