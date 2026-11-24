#!/bin/bash
# FGA0003 - Compiladores 1
# Curso de Engenharia de Software
# Universidade de Brasília (UnB)
# run_all_tests.sh - Roda todas as baterias de testes e mostra o total geral
#
# Chamado pelo "make test". Cada bateria continua com o próprio resumo; no
# fim aparece uma tabela com todas e o total. Uma bateria que falha não
# interrompe as seguintes, então o resumo final sempre sai completo.

DIR_TESTES="$(cd "$(dirname "$0")" && pwd)"

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
AMARELO='\033[1;33m'
RESET='\033[0m'

# Nome exibido | script
BATERIAS=(
    "Compilador (AST, erros, tabela)|run_tests.sh"
    "Construção da AST|run_ast_tests.sh"
    "Scanner|run_scanner_tests.sh"
    "Código intermediário (TAC)|run_tac_tests.sh"
)

SAIDA=$(mktemp)
trap 'rm -f "$SAIDA"' EXIT

TOTAL=0
PASSOU=0
FALHOU=0
LINHAS=()

for item in "${BATERIAS[@]}"; do
    NOME="${item%%|*}"
    SCRIPT="${item##*|}"

    # Mostra a saída normalmente e guarda uma cópia para ler o resultado
    "$DIR_TESTES/$SCRIPT" 2>&1 | tee "$SAIDA"
    STATUS=${PIPESTATUS[0]}

    # Remove as cores e procura "Resultado: N testes | P passaram | F falharam"
    RESULTADO=$(sed 's/\x1b\[[0-9;]*m//g' "$SAIDA" | grep -E '^Resultado: [0-9]+ testes' | tail -n 1)

    if [ -n "$RESULTADO" ]; then
        N=$(echo "$RESULTADO" | sed -E 's/^Resultado: ([0-9]+) testes.*/\1/')
        P=$(echo "$RESULTADO" | sed -E 's/.*\| ([0-9]+) passaram.*/\1/')
        F=$(echo "$RESULTADO" | sed -E 's/.*\| ([0-9]+) falharam.*/\1/')
    else
        # Baterias de um teste só (AST e scanner): vale o código de saída
        N=1
        if [ "$STATUS" -eq 0 ]; then P=1; F=0; else P=0; F=1; fi
    fi

    TOTAL=$((TOTAL + N))
    PASSOU=$((PASSOU + P))
    FALHOU=$((FALHOU + F))
    # Números primeiro: o printf conta bytes, e os acentos desalinhariam
    LINHAS+=("$(printf '%4d testes | %4d ok | %3d falhas | ' "$N" "$P" "$F")$NOME")
done

echo ""
echo -e "${AMARELO}=======================================================${RESET}"
echo -e "${AMARELO}                 RESUMO GERAL DOS TESTES${RESET}"
echo -e "${AMARELO}=======================================================${RESET}"
for linha in "${LINHAS[@]}"; do
    echo "$linha"
done
echo -e "${AMARELO}-------------------------------------------------------${RESET}"
echo -e "TOTAL: $TOTAL testes | ${VERDE}$PASSOU passaram${RESET} | ${VERMELHO}$FALHOU falharam${RESET}"
echo -e "${AMARELO}=======================================================${RESET}"

if [ "$FALHOU" -gt 0 ]; then
    echo -e "${VERMELHO}Há testes falhando.${RESET}"
    exit 1
fi
echo -e "${VERDE}Todos os testes passaram.${RESET}"
exit 0
