/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 10 - prática de geração de código final
 * gerador_js.c - Geração de código JavaScript a partir da AST
 *
 * Mesma ideia do gerador de TAC: cada expressão é traduzida de baixo para
 * cima e devolve uma string (alocada com malloc, liberada por quem recebe)
 * com o código JS equivalente. As instruções são emitidas uma por linha.
 *
 * Simulação dos tipos do C (o tipo do resultado vem de no->tipo_c):
 *   int     + e -       ((a + b) | 0)       "| 0" trunca para 32 bits
 *           *           Math.imul(a, b)     multiplicação exata em 32 bits
 *           /           ((a / b) | 0)       divisão inteira, trunca
 *   long    + - *       (a + b)             sem truncamento (limitação:
 *           /           Math.trunc(a / b)    exato só até 2^53)
 *   float   qualquer    Math.fround(a + b)  arredonda para precisão float
 *   double  qualquer    (a + b)             já é o número do JS
 *   comparação          (a < b ? 1 : 0)     no C o resultado é int 1/0
 * Toda expressão sai entre parênteses, então a precedência é a da AST.
 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "gerador_js.h"

static FILE *saida_js;
static int nivel_indentacao; /* níveis de 4 espaços (usado pelos blocos) */

static char *gerar_expressao(const NoAST *no);
static void gerar_instrucao(const NoAST *no);

/* sprintf que aloca o tamanho certo (asprintf não existe no MinGW) */
static char *formatar(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int tamanho = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    char *buf = malloc((size_t)tamanho + 1);
    va_start(args, fmt);
    vsnprintf(buf, (size_t)tamanho + 1, fmt, args);
    va_end(args);
    return buf;
}

/* Emite uma linha de código com a indentação atual */
static void emitir_linha(const char *fmt, ...)
{
    for (int i = 0; i < nivel_indentacao; i++)
        fputs("    ", saida_js);

    va_list args;
    va_start(args, fmt);
    vfprintf(saida_js, fmt, args);
    va_end(args);
    fputc('\n', saida_js);
}

/* char e int: aritmética em 32 bits com sinal */
static int eh_int32(int tipo)
{
    return tipo == TIPO_CHAR || tipo == TIPO_INT;
}

/* Literal: inteiros (char vira o código numérico) e reais. Um literal
 * float passa por Math.fround para ter a mesma precisão do C. */
static char *texto_numero(const NoAST *no)
{
    if (!tipo_eh_real(no->tipo_c))
        return formatar("%ld", no->valor.i);

    /* Menor representação que volta ao mesmo double (0.1, e não
     * 0.10000000000000001) */
    char buf[64];
    snprintf(buf, sizeof buf, "%.15g", no->valor.r);
    if (strtod(buf, NULL) != no->valor.r)
        snprintf(buf, sizeof buf, "%.17g", no->valor.r);
    /* 2.0 continua com ponto, para não parecer um inteiro no código */
    if (!strpbrk(buf, ".eEnN"))
        strcat(buf, ".0");
    if (no->tipo_c == TIPO_FLOAT)
        return formatar("Math.fround(%s)", buf);
    return strdup(buf);
}

/* Converte 'expr' (do tipo 'origem') para o tipo 'destino', como numa
 * atribuição em C. Real -> inteiro não aparece: a semântica rejeita. */
static char *converter(char *expr, int origem, int destino)
{
    char *convertida = NULL;

    if (destino == origem)
        return expr;

    if (destino == TIPO_CHAR)
        convertida = formatar("((%s << 24) >> 24)", expr); /* 8 bits com sinal */
    else if (destino == TIPO_INT && origem == TIPO_LONG)
        convertida = formatar("(%s | 0)", expr);           /* 32 bits */
    else if (destino == TIPO_FLOAT)
        convertida = formatar("Math.fround(%s)", expr);    /* precisão float */
    else
        return expr; /* promoção: o valor já cabe no tipo de destino */

    free(expr);
    return convertida;
}

static const char *operador_comparacao(TipoNoAST tipo)
{
    switch (tipo)
    {
    case AST_IGUAL:       return "==="; /* tudo é número: === é seguro */
    case AST_DIFERENTE:   return "!==";
    case AST_MENOR:       return "<";
    case AST_MENOR_IGUAL: return "<=";
    case AST_MAIOR:       return ">";
    case AST_MAIOR_IGUAL: return ">=";
    default:              return NULL;
    }
}

static const char *operador_aritmetico(TipoNoAST tipo)
{
    switch (tipo)
    {
    case AST_SOMA:          return "+";
    case AST_SUBTRACAO:     return "-";
    case AST_MULTIPLICACAO: return "*";
    case AST_DIVISAO:       return "/";
    default:                return NULL;
    }
}

/* Operação aritmética binária, conforme o tipo do resultado */
static char *gerar_aritmetica(const NoAST *no, const char *op,
                              const char *esq, const char *dir)
{
    int tipo = no->tipo_c;

    if (eh_int32(tipo))
    {
        if (no->tipo == AST_MULTIPLICACAO)
            return formatar("Math.imul(%s, %s)", esq, dir);
        return formatar("((%s %s %s) | 0)", esq, op, dir);
    }
    if (tipo == TIPO_LONG && no->tipo == AST_DIVISAO)
        return formatar("Math.trunc(%s / %s)", esq, dir);
    if (tipo == TIPO_FLOAT)
        return formatar("Math.fround(%s %s %s)", esq, op, dir);

    return formatar("(%s %s %s)", esq, op, dir); /* long e double */
}

static char *gerar_expressao(const NoAST *no)
{
    if (no->tipo == AST_NUMERO)
        return texto_numero(no);
    if (no->tipo == AST_IDENTIFICADOR)
        return ast_nome_variavel(no);

    if (no->tipo == AST_MENOS_UNARIO)
    {
        char *operando = gerar_expressao(no->esq);
        char *resultado = eh_int32(no->tipo_c)
                              ? formatar("(-%s | 0)", operando)
                              : formatar("(-%s)", operando);
        free(operando);
        return resultado;
    }

    char *esq = gerar_expressao(no->esq);
    char *dir = gerar_expressao(no->dir);
    char *resultado = NULL;
    const char *op;

    if ((op = operador_aritmetico(no->tipo)) != NULL)
        resultado = gerar_aritmetica(no, op, esq, dir);
    else if ((op = operador_comparacao(no->tipo)) != NULL)
        resultado = formatar("(%s %s %s ? 1 : 0)", esq, op, dir);

    free(esq);
    free(dir);
    return resultado;
}

/* Gera a expressão já convertida para o tipo da variável de destino */
static char *gerar_valor(const NoAST *expressao, int tipo_destino)
{
    return converter(gerar_expressao(expressao), expressao->tipo_c,
                     tipo_destino);
}

static void gerar_instrucao(const NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:
    {
        /* Em C a variável não inicializada tem lixo; em JS seria
         * undefined, que estraga contas. Começa em 0. */
        char *nome = ast_nome_variavel(no);
        char *valor = no->esq ? gerar_valor(no->esq, no->tipo_c)
                              : strdup("0");
        emitir_linha("let %s = %s;", nome, valor);
        free(nome);
        free(valor);
        break;
    }

    case AST_ATRIBUICAO:
    {
        int tipo_destino = no->simbolo ? (int)no->simbolo->tipo : no->tipo_c;
        char *nome = ast_nome_variavel(no);
        char *valor = gerar_valor(no->esq, tipo_destino);
        emitir_linha("%s = %s;", nome, valor);
        free(nome);
        free(valor);
        break;
    }

    case AST_EXPRESSAO:
    {
        char *valor = gerar_expressao(no->esq);
        emitir_linha("%s;", valor);
        free(valor);
        break;
    }

    case AST_BLOCO:
    case AST_IF:
    case AST_WHILE:
        /* Controle de fluxo entra na issue #52 */
        fprintf(stderr, "Aviso [Linha %d, Col %d]: geração de JavaScript "
                        "para blocos, if/else e while ainda não implementada\n",
                no->linha, no->coluna);
        break;

    default:
        break;
    }
}

void gerador_js_gerar(const NoAST *raiz, FILE *saida)
{
    if (raiz == NULL)
        return;

    saida_js = saida;
    nivel_indentacao = 0;

    /* Modo estrito: uso de variável não declarada vira erro no JS */
    emitir_linha("\"use strict\";");

    for (const NoAST *no = raiz->esq; no != NULL; no = no->proximo)
        gerar_instrucao(no);
}
