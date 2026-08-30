#!/bin/bash

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
AMARELO='\033[1;33m'
RESET='\033[0m'

EXECUTAVEL="./hello"
DIR_TESTES="."

echo -e "${AMARELO}==> Rodando bateria de testes...${RESET}\n"

if [ ! -f "$EXECUTAVEL" ]; then
    echo -e "${VERMELHO}[ERRO] O binário '$EXECUTAVEL' não foi encontrado.${RESET}"
    echo "Dica: compile o projeto antes de rodar os testes."
    exit 1
fi

TOTAL=0
PASSOU=0
FALHOU=0
FALHAS=()

# Procura arquivos .txt e .in
for arq in "$DIR_TESTES"/*.txt "$DIR_TESTES"/*.in; do
    [ -e "$arq" ] || continue

    TOTAL=$((TOTAL + 1))
    echo -n "• Testando $arq... "

    SAIDA=$($EXECUTAVEL < "$arq" 2>&1)
    STATUS=$?

    if [ $STATUS -eq 0 ]; then
        echo -e "${VERDE}OK${RESET}"
        PASSOU=$((PASSOU + 1))
    else
        echo -e "${VERMELHO}FALHOU${RESET}"
        FALHOU=$((FALHOU + 1))
        FALHAS+=("$arq (status: $STATUS)")
        echo -e "  └─ Saída:\n$SAIDA\n"
    fi
done

echo -e "\n${AMARELO}---------------------------------------${RESET}"

if [ $TOTAL -eq 0 ]; then
    echo -e "${AMARELO}Nenhum arquivo de teste (.txt ou .in) encontrado na pasta.${RESET}"
    echo -e "${AMARELO}---------------------------------------${RESET}\n"
    exit 0
fi

echo -e "Resultado: $TOTAL testes | ${VERDE}${PASSOU} passaram${RESET} | ${VERMELHO}${FALHOU} falharam${RESET}"
echo -e "${AMARELO}---------------------------------------${RESET}"

if [ $FALHOU -gt 0 ]; then
    echo -e "\n${VERMELHO}Arquivos que deram problema:${RESET}"
    for falha in "${FALHAS[@]}"; do
        echo -e "${VERMELHO}  ✖ $falha${RESET}"
    done
    echo ""
    exit 1
else
    echo -e "${VERDE}Tudo limpo! Todos os testes passaram.${RESET}\n"
    exit 0
fi