#!/bin/bash
# FGA0003 - Compiladores 1
# Curso de Engenharia de Software
# Universidade de Brasília (UnB)
# run_js_tests.sh - Testes de ponta a ponta do JavaScript gerado
#
# Para cada programa em entradas_js/:
#   parser --js programa  ->  .js temporário  ->  node  ->  diff com respostas_js/
# A resposta esperada é a saída do MESMO programa compilado com gcc
# (gerada por gerar_respostas_js.sh). Cada programa roda duas vezes: sem e
# com --otimizar, para provar que a otimização não muda o resultado.
#
# Requer Node.js. Sem ele, os testes são pulados (sem falhar o make test).

cd "$(dirname "$0")" || exit 1

EXECUTAVEL="../src/parser"
[ -f "$EXECUTAVEL" ] || EXECUTAVEL="../src/parser.exe"

DIR_ENTRADAS="entradas_js"
DIR_RESPOSTAS="respostas_js"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
PROGRAMA_JS="$TMP/programa.js"
RESULTADO="$TMP/resultado.txt"
ERROS="$TMP/erros.txt"

TOTAL=0
PASSOU=0
FALHOU=0

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
AMARELO='\033[1;33m'
RESET='\033[0m'

echo -e "${AMARELO}==> Testes de ponta a ponta do JavaScript (node)${RESET}\n"

if ! command -v node > /dev/null 2>&1; then
    echo -e "${AMARELO}[AVISO] Node.js não encontrado: testes de JavaScript pulados.${RESET}"
    echo -e "${AMARELO}        Instale o Node.js (https://nodejs.org) para rodá-los.${RESET}\n"
    echo -e "Resultado: 0 testes | ${VERDE}0 passaram${RESET} | ${VERMELHO}0 falharam${RESET}"
    exit 0
fi

if [ ! -f "$EXECUTAVEL" ]; then
    echo -e "${VERMELHO}[ERRO] Executável não encontrado.${RESET}"
    exit 1
fi

falhar() {
    echo -e "${VERMELHO}[FALHA] $1: $2${RESET}"
    [ -s "$ERROS" ] && cat "$ERROS"
    echo ""
    ((FALHOU++))
}

# $1 = nome do teste, $2 = opções extras do parser (ex.: --otimizar)
rodar_teste() {
    local NOME="$1"
    local OPCOES="$2"
    local ROTULO="$NOME${OPCOES:+ ($OPCOES)}"
    local ENTRADA="$DIR_ENTRADAS/$NOME.txt"
    local RESPOSTA="$DIR_RESPOSTAS/$NOME.txt"

    ((TOTAL++))
    echo -e "${AMARELO}Testando: $ROTULO${RESET}"

    if [ ! -f "$RESPOSTA" ]; then
        falhar "$ROTULO" "resposta não encontrada (rode gerar_respostas_js.sh)"
        return
    fi

    # 1. Compila para JavaScript
    if ! "$EXECUTAVEL" --js $OPCOES < "$ENTRADA" > "$PROGRAMA_JS" 2> "$ERROS"; then
        falhar "$ROTULO" "o parser rejeitou o programa"
        return
    fi

    # 2. Executa no node
    if ! node "$PROGRAMA_JS" > "$RESULTADO" 2> "$ERROS"; then
        falhar "$ROTULO" "erro ao executar no node"
        return
    fi

    # 3. Compara com a saída do programa em C
    if diff -u "$RESPOSTA" "$RESULTADO" > /dev/null; then
        echo -e "${VERDE}[OK] $ROTULO passou.${RESET}\n"
        ((PASSOU++))
    else
        : > "$ERROS"
        falhar "$ROTULO" "saída diferente da do C"
        diff -u "$RESPOSTA" "$RESULTADO"
        echo ""
    fi
}

for ENTRADA in "$DIR_ENTRADAS"/*.txt; do
    [ -e "$ENTRADA" ] || continue
    NOME=$(basename "$ENTRADA" .txt)
    rodar_teste "$NOME" ""
    rodar_teste "$NOME" "--otimizar"
done

echo -e "${AMARELO}---------------------------------------${RESET}"
echo -e "Resultado: $TOTAL testes | ${VERDE}$PASSOU passaram${RESET} | ${VERMELHO}$FALHOU falharam${RESET}"
echo -e "${AMARELO}---------------------------------------${RESET}"

[ "$FALHOU" -eq 0 ] && exit 0 || exit 1
