#!/bin/bash

cd "$(dirname "$0")" || exit 1
shopt -s nullglob

VERDE='\033[0;32m'
VERMELHO='\033[0;31m'
AMARELO='\033[1;33m'
RESET='\033[0m'

EXECUTAVEL="../src/parser"
RESULTADO="/tmp/resultado.txt"
ERROS="/tmp/erros.txt"
RELATORIO="relatorio.txt"

# Pastas dos dois tipos de teste
DIR_TESTES="entradas"
DIR_RESPOSTAS="respostas"
DIR_TESTES_ERRO="entradas_erro"
DIR_RESPOSTAS_ERRO="respostas_erro"

echo -e "${AMARELO}==> Rodando bateria de testes...${RESET}\n"

if [ ! -f "$EXECUTAVEL" ]; then
    echo -e "${VERMELHO}[ERRO] O binário '$EXECUTAVEL' não foi encontrado.${RESET}"
    echo "Dica: compile o projeto antes de rodar os testes."
    exit 1
fi

{
    echo "Relatório de testes - $(date '+%Y-%m-%d %H:%M:%S')"
    echo "========================================"
    echo ""
} > "$RELATORIO"

TOTAL=0
PASSOU=0
FALHOU=0
IGNORADOS=0
FALHAS=()

# ---------- Testes que devem PASSAR (compara stdout) ----------
for ARQUIVO_ENTRADA in "$DIR_TESTES"/*.txt; do
    [ -e "$ARQUIVO_ENTRADA" ] || continue

    NOME_TESTE=$(basename "$ARQUIVO_ENTRADA" .txt)
    ARQUIVO_RESPOSTA="$DIR_RESPOSTAS/$NOME_TESTE.txt"

    if [ ! -f "$ARQUIVO_RESPOSTA" ]; then
        echo -e "${VERMELHO}[ERRO] Sem resposta para '$NOME_TESTE'.${RESET}"
        ((IGNORADOS++)); continue
    fi

    echo -e "${AMARELO}Rodando teste (saída): $NOME_TESTE${RESET}"
    "$EXECUTAVEL" < "$ARQUIVO_ENTRADA" > "$RESULTADO" 2> "$ERROS"
    STATUS=$?

    if diff -q "$RESULTADO" "$ARQUIVO_RESPOSTA" > /dev/null; then
        echo -e "${VERDE}[OK] '$NOME_TESTE' passou.${RESET}\n"
        ((PASSOU++))
    else
        echo -e "${VERMELHO}[FALHA] '$NOME_TESTE' falhou.${RESET}\n"
        FALHAS+=("$NOME_TESTE (saída)")
        ((FALHOU++))
        {
            echo "----------------------------------------"
            echo "TESTE DE SAÍDA FALHOU: $NOME_TESTE"
            echo "Código de saída: $STATUS"
            echo ""
            echo "--- Entrada ---"; cat "$ARQUIVO_ENTRADA"; echo ""
            echo "--- Esperado (stdout) ---"; cat "$ARQUIVO_RESPOSTA"; echo ""
            echo "--- Obtido (stdout) ---"; cat "$RESULTADO"; echo ""
            echo "--- Diferenças ---"; diff "$ARQUIVO_RESPOSTA" "$RESULTADO"; echo ""
            echo "--- stderr capturado ---"
            [ -s "$ERROS" ] && cat "$ERROS" || echo "(vazio)"
            echo ""
        } >> "$RELATORIO"
    fi
    ((TOTAL++))
done

# ---------- Testes que devem FALHAR (compara stderr) ----------
for ARQUIVO_ENTRADA in "$DIR_TESTES_ERRO"/*.txt; do
    [ -e "$ARQUIVO_ENTRADA" ] || continue

    NOME_TESTE=$(basename "$ARQUIVO_ENTRADA" .txt)
    ARQUIVO_RESPOSTA="$DIR_RESPOSTAS_ERRO/$NOME_TESTE.txt"

    if [ ! -f "$ARQUIVO_RESPOSTA" ]; then
        echo -e "${VERMELHO}[ERRO] Sem resposta de erro para '$NOME_TESTE'.${RESET}"
        ((IGNORADOS++)); continue
    fi

    echo -e "${AMARELO}Rodando teste (erro): $NOME_TESTE${RESET}"
    "$EXECUTAVEL" < "$ARQUIVO_ENTRADA" > "$RESULTADO" 2> "$ERROS"
    STATUS=$?

    # Aqui comparamos o STDERR com a resposta esperada
    if diff -q "$ERROS" "$ARQUIVO_RESPOSTA" > /dev/null; then
        echo -e "${VERDE}[OK] '$NOME_TESTE' produziu o erro esperado.${RESET}\n"
        ((PASSOU++))
    else
        echo -e "${VERMELHO}[FALHA] '$NOME_TESTE' não bateu com o erro esperado.${RESET}\n"
        FALHAS+=("$NOME_TESTE (erro)")
        ((FALHOU++))
        {
            echo "----------------------------------------"
            echo "TESTE DE ERRO FALHOU: $NOME_TESTE"
            echo "Código de saída: $STATUS"
            echo ""
            echo "--- Entrada ---"; cat "$ARQUIVO_ENTRADA"; echo ""
            echo "--- Erro esperado (stderr) ---"; cat "$ARQUIVO_RESPOSTA"; echo ""
            echo "--- Erro obtido (stderr) ---"
            [ -s "$ERROS" ] && cat "$ERROS" || echo "(vazio)"
            echo ""
            echo "--- Diferenças ---"; diff "$ARQUIVO_RESPOSTA" "$ERROS"; echo ""
            echo "--- stdout capturado (deveria estar vazio) ---"
            [ -s "$RESULTADO" ] && cat "$RESULTADO" || echo "(vazio)"
            echo ""
        } >> "$RELATORIO"
    fi
    ((TOTAL++))
done

# ---------- Resumo ----------
echo -e "${AMARELO}---------------------------------------${RESET}"
if [ $TOTAL -eq 0 ] && [ $IGNORADOS -eq 0 ]; then
    echo -e "${AMARELO}Nenhum teste encontrado.${RESET}\n"; exit 0
fi
echo -e "Resultado: $TOTAL testes | ${VERDE}${PASSOU} passaram${RESET} | ${VERMELHO}${FALHOU} falharam${RESET}"
[ $IGNORADOS -gt 0 ] && echo -e "${AMARELO}${IGNORADOS} ignorados${RESET}"
echo -e "${AMARELO}---------------------------------------${RESET}"

if [ $FALHOU -gt 0 ]; then
    echo -e "\n${VERMELHO}Testes que falharam:${RESET}"
    for falha in "${FALHAS[@]}"; do
        echo -e "${VERMELHO}  ✖ $falha${RESET}"
    done
    echo -e "\n${AMARELO}Detalhes em: $RELATORIO${RESET}\n"
    exit 1
else
    echo -e "\n${VERDE}Tudo limpo! Todos os testes passaram.${RESET}\n"
    exit 0
fi