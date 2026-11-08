/* FGA0003 - Compiladores 1
 * ast.c - Construção, impressão e liberação da AST (ver ast.h)
 */
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* ---------- Criação ---------- */

static void sem_memoria(void)
{
    fprintf(stderr, "Erro: Falha na alocação de memória para a AST.\n");
    exit(1);
}

static NoAST *criar_no_base(TipoNoAST tipo, int linha, int coluna)
{
    NoAST *no = calloc(1, sizeof(NoAST));
    if (!no)
        sem_memoria();

    no->tipo = tipo;
    no->tipo_c = TIPO_INEXISTENTE;
    no->linha = linha;
    no->coluna = coluna;

    return no;
}

static char *copiar_texto(const char *s)
{
    char *copia = strdup(s);
    if (!copia)
        sem_memoria();

    return copia;
}

NoAST *ast_criar_numero_int(int tipo_c, long valor, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_NUMERO, linha, coluna);
    no->tipo_c = tipo_c;
    no->valor.i = valor;
    return no;
}

NoAST *ast_criar_numero_real(int tipo_c, double valor, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_NUMERO, linha, coluna);
    no->tipo_c = tipo_c;
    no->valor.r = valor;
    return no;
}

NoAST *ast_criar_identificador(const char *nome, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_IDENTIFICADOR, linha, coluna);
    no->texto = copiar_texto(nome);
    return no;
}

NoAST *ast_criar_operacao(TipoNoAST tipo, NoAST *esq, NoAST *dir,
                          int linha, int coluna)
{
    NoAST *no = criar_no_base(tipo, linha, coluna);
    no->esq = esq;
    no->dir = dir;
    return no;
}

NoAST *ast_criar_menos_unario(NoAST *operando, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_MENOS_UNARIO, linha, coluna);
    no->esq = operando;
    return no;
}

NoAST *ast_criar_declaracao(int tipo_c, const char *nome,
                            NoAST *inicializacao, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_DECLARACAO, linha, coluna);
    no->tipo_c = tipo_c;
    no->texto = copiar_texto(nome);
    no->esq = inicializacao;
    return no;
}

NoAST *ast_criar_atribuicao(const char *nome, NoAST *expressao,
                            int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_ATRIBUICAO, linha, coluna);
    no->texto = copiar_texto(nome);
    no->esq = expressao;
    return no;
}

NoAST *ast_criar_expressao(NoAST *expressao, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_EXPRESSAO, linha, coluna);
    no->esq = expressao;
    return no;
}

NoAST *ast_criar_bloco(NoAST *instrucoes)
{
    NoAST *no = criar_no_base(AST_BLOCO, 0, 0);
    no->esq = instrucoes;
    return no;
}

NoAST *ast_criar_if(NoAST *condicao, NoAST *bloco_entao,
                    NoAST *bloco_senao, int linha, int coluna)
{
    NoAST *no = criar_no_base(AST_IF, linha, coluna);
    no->esq = condicao;
    no->dir = bloco_entao;
    no->terceiro = bloco_senao;
    return no;
}

NoAST *ast_criar_programa(NoAST *instrucoes)
{
    NoAST *no = criar_no_base(AST_PROGRAMA, 1, 1);
    no->esq = instrucoes;
    return no;
}

NoAST *ast_anexar(NoAST *lista, NoAST *no)
{
    if (!lista)
        return no;

    NoAST *ultimo = lista;

    while (ultimo->proximo)
        ultimo = ultimo->proximo;

    ultimo->proximo = no;
    return lista;
}

/* ---------- Impressão ---------- */

static const char *nome_no(TipoNoAST tipo)
{
    switch (tipo)
    {
    case AST_PROGRAMA:      return "AST_PROGRAMA";
    case AST_DECLARACAO:    return "AST_DECLARACAO";
    case AST_ATRIBUICAO:    return "AST_ATRIBUICAO";
    case AST_EXPRESSAO:     return "AST_EXPRESSAO";
    case AST_BLOCO:         return "AST_BLOCO";
    case AST_IF:            return "AST_IF";

    case AST_NUMERO:        return "AST_NUMERO";
    case AST_IDENTIFICADOR: return "AST_IDENTIFICADOR";

    case AST_SOMA:          return "AST_SOMA";
    case AST_SUBTRACAO:     return "AST_SUBTRACAO";
    case AST_MULTIPLICACAO: return "AST_MULTIPLICACAO";
    case AST_DIVISAO:       return "AST_DIVISAO";
    case AST_MENOS_UNARIO:  return "AST_MENOS_UNARIO";

    case AST_IGUAL:         return "AST_IGUAL";
    case AST_DIFERENTE:     return "AST_DIFERENTE";
    case AST_MENOR:         return "AST_MENOR";
    case AST_MENOR_IGUAL:   return "AST_MENOR_IGUAL";
    case AST_MAIOR:         return "AST_MAIOR";
    case AST_MAIOR_IGUAL:   return "AST_MAIOR_IGUAL";

    default:                return "AST_DESCONHECIDO";
    }
}

static void imprimir_numero(const NoAST *no)
{
    switch (no->tipo_c)
    {
    case TIPO_CHAR:
        if (isprint((unsigned char)no->valor.i))
            printf("'%c'", (char)no->valor.i);
        else
            printf("%ld", no->valor.i);
        break;

    case TIPO_LONG:
        printf("%ldL", no->valor.i);
        break;

    case TIPO_FLOAT:
    case TIPO_DOUBLE:
    {
        char buf[64];
        snprintf(buf, sizeof buf, "%g", no->valor.r);

        if (!strpbrk(buf, ".eEnN"))
            strcat(buf, ".0");

        printf("%s%s", buf, no->tipo_c == TIPO_FLOAT ? "f" : "");
        break;
    }

    default:
        printf("%ld", no->valor.i);
        break;
    }
}

void ast_imprimir(const NoAST *raiz, int nivel)
{
    for (const NoAST *no = raiz; no; no = no->proximo)
    {
        for (int i = 0; i < nivel; i++)
            printf("  ");

        printf("%s", nome_no(no->tipo));

        switch (no->tipo)
        {
        case AST_DECLARACAO:
            printf(" (%s %s)", nome_tipo(no->tipo_c), no->texto);
            break;

        case AST_ATRIBUICAO:
        case AST_IDENTIFICADOR:
            printf(" (%s)", no->texto);
            break;

        case AST_NUMERO:
            printf(" (");
            imprimir_numero(no);
            printf(")");
            break;

        default:
            break;
        }

        printf("\n");

        ast_imprimir(no->esq, nivel + 1);
        ast_imprimir(no->dir, nivel + 1);
        ast_imprimir(no->terceiro, nivel + 1);
    }
}

/* ---------- Liberação ---------- */

void ast_liberar(NoAST *raiz)
{
    while (raiz)
    {
        NoAST *proximo = raiz->proximo;

        ast_liberar(raiz->esq);
        ast_liberar(raiz->dir);
        ast_liberar(raiz->terceiro);

        free(raiz->texto);
        free(raiz);

        raiz = proximo;
    }
}