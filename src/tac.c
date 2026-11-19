/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 8 - prática de geração de código intermediário
 * tac.c - Geração de código de três endereços (TAC) a partir da AST
 *
 * Segue a ideia do protótipo da Semana 8 (gerarTAC), adaptada à AST real
 * do projeto:
 *   1. folha (número ou identificador): devolve o próprio "endereço",
 *      sem emitir instrução;
 *   2. operação: gera o filho esquerdo, depois o direito, cria uma
 *      temporária nova e emite  tN = esq op dir;
 *   3. atribuição: tratada ANTES da regra geral (correção pedida em aula),
 *      gera a expressão e emite  x = valor  direto, sem temporária extra.
 *
 * Todo "endereço" devolvido é uma string alocada com malloc; quem recebe
 * é responsável por liberá-la depois de usar.
 */
#include <stdlib.h>
#include <string.h>
#include "tac.h"

/* Contador das temporárias t0, t1, ... (zerado em tac_gerar) */
static int contador_temp;
static FILE *saida_tac;

static char *gerar_expressao(const NoAST *no);
static void gerar_instrucao(const NoAST *no);

/* Cria o nome da próxima temporária: t0, t1, t2, ... */
static char *nova_temp(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "t%d", contador_temp++);
    return strdup(buf);
}

/* Texto de um literal. Inteiros (char/int/long) saem como número;
 * reais sempre com ponto, para 5.0 não virar o inteiro 5. */
static char *texto_numero(const NoAST *no)
{
    char buf[64];

    if (tipo_eh_real(no->tipo_c))
    {
        snprintf(buf, sizeof buf, "%g", no->valor.r);
        if (!strpbrk(buf, ".eEnN"))
            strcat(buf, ".0");
    }
    else
    {
        snprintf(buf, sizeof buf, "%ld", no->valor.i);
    }
    return strdup(buf);
}

/* Símbolo do operador no TAC, ou NULL se o nó não é operação binária */
static const char *operador_binario(TipoNoAST tipo)
{
    switch (tipo)
    {
    case AST_SOMA:          return "+";
    case AST_SUBTRACAO:     return "-";
    case AST_MULTIPLICACAO: return "*";
    case AST_DIVISAO:       return "/";
    case AST_IGUAL:         return "==";
    case AST_DIFERENTE:     return "!=";
    case AST_MENOR:         return "<";
    case AST_MENOR_IGUAL:   return "<=";
    case AST_MAIOR:         return ">";
    case AST_MAIOR_IGUAL:   return ">=";
    default:                return NULL;
    }
}

/* Gera o TAC de uma expressão e devolve onde está o resultado:
 * o nome de uma variável, um literal ou uma temporária. */
static char *gerar_expressao(const NoAST *no)
{
    if (no == NULL)
        return NULL;

    /* 1. Folhas: não emitem instrução */
    if (no->tipo == AST_NUMERO)
        return texto_numero(no);
    if (no->tipo == AST_IDENTIFICADOR)
        return strdup(no->texto);

    /* Menos unário: tN = - operando */
    if (no->tipo == AST_MENOS_UNARIO)
    {
        char *operando = gerar_expressao(no->esq);
        char *temp = nova_temp();
        fprintf(saida_tac, "%s = - %s\n", temp, operando);
        free(operando);
        return temp;
    }

    /* 2. Operação binária: esquerda, direita, depois emite */
    const char *op = operador_binario(no->tipo);
    if (op != NULL)
    {
        char *esq = gerar_expressao(no->esq);
        char *dir = gerar_expressao(no->dir);
        char *temp = nova_temp();
        fprintf(saida_tac, "%s = %s %s %s\n", temp, esq, op, dir);
        free(esq);
        free(dir);
        return temp;
    }

    return NULL; /* não acontece numa AST válida */
}

/* Destino = expressão, sem criar temporária para a cópia */
static void gerar_copia(const char *destino, const NoAST *expressao)
{
    char *valor = gerar_expressao(expressao);
    fprintf(saida_tac, "%s = %s\n", destino, valor);
    free(valor);
}

static void gerar_instrucao(const NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:
        /* Só a inicialização vira código; "int x;" não emite nada */
        if (no->esq != NULL)
            gerar_copia(no->texto, no->esq);
        break;

    case AST_ATRIBUICAO:
        /* 3. Correção da Semana 8: x = t1, e não t2 = x = t1 */
        gerar_copia(no->texto, no->esq);
        break;

    case AST_EXPRESSAO:
        /* "a + b;": as operações são geradas, o resultado é descartado */
        free(gerar_expressao(no->esq));
        break;

    case AST_BLOCO:
    case AST_IF:
        /* Controle de fluxo entra na issue #42 */
        fprintf(stderr, "Aviso [Linha %d, Col %d]: geração de TAC para "
                        "if/else e blocos ainda não implementada\n",
                no->linha, no->coluna);
        break;

    default:
        break;
    }
}

void tac_gerar(const NoAST *raiz, FILE *saida)
{
    if (raiz == NULL)
        return;

    contador_temp = 0;
    saida_tac = saida;

    /* AST_PROGRAMA: instruções encadeadas por 'proximo' a partir de esq */
    for (const NoAST *no = raiz->esq; no != NULL; no = no->proximo)
        gerar_instrucao(no);
}
