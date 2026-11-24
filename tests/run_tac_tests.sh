#!/bin/bash

cd "$(dirname "$0")" || exit 1

EXECUTAVEL="../src/parser"
[ -f "$EXECUTAVEL" ] || EXECUTAVEL="../src/parser.exe"

DIR_ENTRADAS="entradas_tac"
DIR_RESPOSTAS="respostas_tac"

RESULTADO="/tmp/resultado_tac.txt"
ERROS="/tmp/erros_tac.txt"

TOTAL=0
PASSOU=0
FALHOU=0

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
AMARELO='\033[1;33m'
RESET='\033[0m'

echo -e "${AMARELO}==> Testes de geração de TAC${RESET}\n"

if [ ! -f "$EXECUTAVEL" ]; then
    echo -e "${VERMELHO}[ERRO] Executável não encontrado.${RESET}"
    exit 1
fi

for ENTRADA in "$DIR_ENTRADAS"/*.txt; do
    [ -e "$ENTRADA" ] || continue

    NOME=$(basename "$ENTRADA" .txt)
    RESPOSTA="$DIR_RESPOSTAS/$NOME.txt"

    if [ ! -f "$RESPOSTA" ]; then
        echo -e "${VERMELHO}[ERRO] Resposta não encontrada: $NOME${RESET}"
        ((FALHOU++))
        ((TOTAL++))
        continue
    fi

    echo -e "${AMARELO}Testando: $NOME${RESET}"

    "$EXECUTAVEL" --tac < "$ENTRADA" > "$RESULTADO" 2> "$ERROS"

    if diff -u "$RESPOSTA" "$RESULTADO" > /dev/null; then
        echo -e "${VERDE}[OK] $NOME passou.${RESET}\n"
        ((PASSOU++))
    else
        echo -e "${VERMELHO}[FALHA] $NOME falhou.${RESET}"
        diff -u "$RESPOSTA" "$RESULTADO"
        echo ""
        if [ -s "$ERROS" ]; then
            echo "Erros:"
            cat "$ERROS"
        fi
        echo ""
        ((FALHOU++))
    fi

    ((TOTAL++))
done

echo -e "${AMARELO}---------------------------------------${RESET}"
echo -e "Resultado: $TOTAL testes | ${VERDE}$PASSOU passaram${RESET} | ${VERMELHO}$FALHOU falharam${RESET}"
echo -e "${AMARELO}---------------------------------------${RESET}"

[ "$FALHOU" -eq 0 ] && exit 0 || exit 1
