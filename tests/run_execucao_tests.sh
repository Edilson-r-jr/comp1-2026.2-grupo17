#!/bin/bash

cd "$(dirname "$0")/.." || exit 1

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
RESET='\033[0m'

passaram=0
falharam=0

echo "Executando testes de execução..."
echo

make -C src

if [ $? -ne 0 ]; then
    echo "Erro ao compilar o projeto."
    exit 1
fi

for entrada in tests/entradas_execucao/*.txt; do
    nome=$(basename "$entrada")
    resposta="tests/respostas_execucao/$nome"

    if [ ! -f "$resposta" ]; then
        echo -e "${VERMELHO}FALHOU${RESET}: $nome (resposta esperada não encontrada)"
        ((falharam++))
        continue
    fi

    if ! src/parser < "$entrada" > /tmp/resultado_execucao.txt 2> /tmp/erro_execucao.txt; then
        echo -e "${VERMELHO}FALHOU${RESET}: $nome (erro durante a execução)"
        cat /tmp/erro_execucao.txt
        ((falharam++))
        continue
    fi

    if [ -s /tmp/erro_execucao.txt ]; then
        echo -e "${VERMELHO}FALHOU${RESET}: $nome (erro durante a execução)"
        cat /tmp/erro_execucao.txt
        ((falharam++))
        continue
    fi

    resultado=$(cat /tmp/resultado_execucao.txt)
    esperado=$(cat "$resposta")

    if [ "$resultado" = "$esperado" ]; then
        echo -e "${VERDE}PASSOU${RESET}: $nome"
        ((passaram++))
    else
        echo -e "${VERMELHO}FALHOU${RESET}: $nome"
        echo "Resultado esperado:"
        cat "$resposta"
        echo
        echo "Resultado obtido:"
        cat /tmp/resultado_execucao.txt
        ((falharam++))
    fi
done

rm -f /tmp/resultado_execucao.txt /tmp/erro_execucao.txt

echo
echo "================================"
echo "Resultado dos testes de execução"
echo "================================"
echo "Total: $((passaram + falharam))"
echo "Passaram: $passaram"
echo "Falharam: $falharam"

if [ "$falharam" -gt 0 ]; then
    exit 1
fi

exit 0