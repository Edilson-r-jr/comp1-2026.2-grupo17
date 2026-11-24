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
static int contador_rotulo;

static char *gerar_expressao(const NoAST *no);
static void gerar_instrucao(const NoAST *no);
static char *novo_rotulo(void);
static void gerar_if(const NoAST *no);
static void gerar_while(const NoAST *no);
static void gerar_bloco(const NoAST *bloco);

/* while (cond) { corpo }  vira:
 *   Linicio:
 *   <código da condição>
 *   ifFalse cond goto Lfim
 *   <corpo>
 *   goto Linicio
 *   Lfim:
 * A condição fica depois do rótulo de início para ser reavaliada a cada
 * volta. Os dois rótulos são criados antes de gerar qualquer código, então
 * o laço externo recebe os números menores. */
static void gerar_while(const NoAST *no)
{
    char *inicio = novo_rotulo();
    char *fim = novo_rotulo();

    fprintf(saida_tac, "%s:\n", inicio);

    char *condicao = gerar_expressao(no->esq);
    fprintf(saida_tac, "ifFalse %s goto %s\n", condicao, fim);

    gerar_bloco(no->dir);

    fprintf(saida_tac, "goto %s\n", inicio);
    fprintf(saida_tac, "%s:\n", fim);

    free(condicao);
    free(inicio);
    free(fim);
}

static void gerar_if(const NoAST *no)
{
    char *condicao = gerar_expressao(no->esq);

    if (no->terceiro == NULL)
    {
        /* if sem else */
        char *fim = novo_rotulo();

        fprintf(saida_tac, "ifFalse %s goto %s\n", condicao, fim);

        gerar_bloco(no->dir);

        fprintf(saida_tac, "%s:\n", fim);

        free(fim);
    }
    else
    {
        /* if com else */
        char *rotulo_else = novo_rotulo();
        char *fim = novo_rotulo();

        fprintf(saida_tac, "ifFalse %s goto %s\n",
                condicao, rotulo_else);

        gerar_bloco(no->dir);

        fprintf(saida_tac, "goto %s\n", fim);
        fprintf(saida_tac, "%s:\n", rotulo_else);

        gerar_bloco(no->terceiro);

        fprintf(saida_tac, "%s:\n", fim);

        free(rotulo_else);
        free(fim);
    }

    free(condicao);
}

/* Cria o nome da próxima temporária: t0, t1, t2, ... */
static char *nova_temp(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "t%d", contador_temp++);
    return strdup(buf);
}

/* Cria o nome do próximo rótulo: L0, L1, L2, ... */
static char *novo_rotulo(void)
{
    char buf[32];
    snprintf(buf, sizeof buf, "L%d", contador_rotulo++);
    return strdup(buf);
}

/* Retorna o nome da variável considerando seu escopo semântico. */
static char *nome_variavel(const NoAST *no)
{
    if (no->simbolo == NULL)
        return strdup(no->texto);

    if (no->simbolo->escopo == ESCOPO_GLOBAL)
        return strdup(no->simbolo->nome);

    char buf[128];
    snprintf(buf, sizeof buf, "%s_%d",
             no->simbolo->nome, no->simbolo->escopo);

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
        return nome_variavel(no);

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

static void gerar_bloco(const NoAST *bloco)
{
    if (bloco == NULL)
        return;

    for (const NoAST *no = bloco->esq;
         no != NULL;
         no = no->proximo)
    {
        gerar_instrucao(no);
    }
}

static void gerar_instrucao(const NoAST *no)
{
    switch (no->tipo)
    {
    case AST_DECLARACAO:
        /* Só a inicialização vira código; "int x;" não emite nada */
        if (no->esq != NULL)
        {
            char *destino = nome_variavel(no);
            gerar_copia(destino, no->esq);
            free(destino);
        }
        break;

    case AST_ATRIBUICAO:
    {
        /* Usa o símbolo semântico para respeitar o escopo da variável. */
        char *destino = nome_variavel(no);
        gerar_copia(destino, no->esq);
        free(destino);
        break;
    }

    case AST_EXPRESSAO:
        /* "a + b;": as operações são geradas, o resultado é descartado */
        free(gerar_expressao(no->esq));
        break;

    case AST_BLOCO:
        gerar_bloco(no);
        break;

    case AST_IF:
        gerar_if(no);
        break;

    case AST_WHILE:
        gerar_while(no);
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
    contador_rotulo = 0;
    saida_tac = saida;

    /* AST_PROGRAMA: instruções encadeadas por 'proximo' a partir de esq */
    for (const NoAST *no = raiz->esq; no != NULL; no = no->proximo)
        gerar_instrucao(no);
}
