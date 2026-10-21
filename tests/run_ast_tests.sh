#!/bin/bash
# Teste do módulo da AST: compila tests/test_ast.c com src/ast.c,
# compara a árvore impressa com tests/ast/esperado.txt e, se o valgrind
# estiver instalado, verifica que ast_liberar não deixa vazamentos.

cd "$(dirname "$0")/.." || exit 1

BIN=tests/test_ast
SAIDA=$(mktemp)
trap 'rm -f "$BIN" "$SAIDA"' EXIT

if ! gcc -Wall -Wextra -Werror -std=gnu11 -g -Isrc tests/test_ast.c src/ast.c -o "$BIN"; then
    echo "Teste da AST: FALHOU (compilação)"
    exit 1
fi

"$BIN" > "$SAIDA"
if ! diff -u tests/ast/esperado.txt "$SAIDA"; then
    echo "Teste da AST: FALHOU (saída diferente do esperado)"
    exit 1
fi

if command -v valgrind > /dev/null; then
    if ! valgrind -q --leak-check=full --error-exitcode=9 "$BIN" > /dev/null; then
        echo "Teste da AST: FALHOU (valgrind encontrou erros de memória)"
        exit 1
    fi
else
    echo "(valgrind não encontrado: verificação de memória pulada)"
fi

echo "Teste da AST: OK"
