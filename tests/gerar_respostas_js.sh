#!/bin/bash
# FGA0003 - Compiladores 1
# Curso de Engenharia de Software
# Universidade de Brasília (UnB)
# gerar_respostas_js.sh - Gera as respostas esperadas dos testes de JS
# compilando os MESMOS programas com o gcc e executando.
#
# Assim a resposta esperada é o que o C de verdade imprime, e não o que
# achamos que ele imprimiria. Não roda no "make test": use quando criar ou
# alterar um teste em entradas_js/ e confira a resposta antes de commitar.
#
# Uso: ./gerar_respostas_js.sh            (todos os testes)
#      ./gerar_respostas_js.sh nome ...   (só os testes indicados, sem .txt)
#
# Na nossa linguagem as instruções ficam soltas, fora de funções, o que não
# é C válido. Por isso cada programa é "embrulhado": as linhas #include
# ficam no topo e o resto vai para dentro de um main.

cd "$(dirname "$0")" || exit 1

DIR_ENTRADAS="entradas_js"
DIR_RESPOSTAS="respostas_js"

if ! command -v gcc > /dev/null 2>&1; then
    echo "gcc não encontrado: instale o gcc para gerar as respostas." >&2
    exit 1
fi

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

if [ $# -gt 0 ]; then
    NOMES=("$@")
else
    NOMES=()
    for ENTRADA in "$DIR_ENTRADAS"/*.txt; do
        [ -e "$ENTRADA" ] && NOMES+=("$(basename "$ENTRADA" .txt)")
    done
fi

STATUS=0
for NOME in "${NOMES[@]}"; do
    ENTRADA="$DIR_ENTRADAS/$NOME.txt"
    if [ ! -f "$ENTRADA" ]; then
        echo "[ERRO] $ENTRADA não existe" >&2
        STATUS=1
        continue
    fi

    {
        grep -E '^[[:space:]]*#[[:space:]]*include' "$ENTRADA"
        echo "int main(void) {"
        grep -vE '^[[:space:]]*#[[:space:]]*include' "$ENTRADA"
        echo ""
        echo "return 0;"
        echo "}"
    } > "$TMP/programa.c"

    if ! gcc -std=c11 -w -o "$TMP/programa" "$TMP/programa.c" -lm; then
        echo "[ERRO] gcc não compilou $NOME" >&2
        STATUS=1
        continue
    fi

    if "$TMP/programa" > "$DIR_RESPOSTAS/$NOME.txt"; then
        echo "[OK] $DIR_RESPOSTAS/$NOME.txt"
    else
        echo "[ERRO] O programa C falhou ao executar: $NOME" >&2
        rm -f "$DIR_RESPOSTAS/$NOME.txt"
        STATUS=1
    fi
done

exit $STATUS
