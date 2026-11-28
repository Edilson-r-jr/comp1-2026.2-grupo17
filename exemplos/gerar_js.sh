#!/bin/bash
# Gera os arquivos .js a partir dos fontes C usando o parser real do projeto.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PARSER="$ROOT/src/parser"

if [ ! -x "$PARSER" ]; then
    echo "Parser não encontrado. Compile primeiro com: make -C src" >&2
    exit 1
fi

for fonte in "$ROOT"/exemplos/*.c; do
    destino="${fonte%.c}.js"
    echo "Gerando $(basename "$destino")"
    "$PARSER" --js "$fonte" > "$destino"
done

echo "Exemplos JavaScript atualizados."
