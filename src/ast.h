/* FGA0003 - Compiladores 1
 * ast.h - Árvore Sintática Abstrata (AST)
 *
 * Representa a estrutura do programa em memória, separada das ações do
 * Bison. Cada nó tem uma categoria (TipoNoAST), até dois filhos (esq/dir)
 * e um ponteiro 'proximo' que encadeia instruções da mesma lista.
 *
 * Os tipos da linguagem (char, int, long, float, double) vêm do enum Tipo
 * de tabela.h, para que o parser, a tabela de símbolos e a AST usem os
 * mesmos valores. A AST só usa o enum e funções puras (nome_tipo,
 * tipo_eh_real); ela não lê nem altera o estado da tabela.
 */
#ifndef AST_H
#define AST_H

#include "tabela.h"

typedef enum
{
    AST_PROGRAMA,
    AST_DECLARACAO,
    AST_ATRIBUICAO,
    AST_EXPRESSAO,
    AST_BLOCO,
    AST_IF,

    AST_NUMERO,
    AST_IDENTIFICADOR,

    AST_SOMA,
    AST_SUBTRACAO,
    AST_MULTIPLICACAO,
    AST_DIVISAO,
    AST_MENOS_UNARIO,

    AST_IGUAL,
    AST_DIFERENTE,
    AST_MENOR,
    AST_MENOR_IGUAL,
    AST_MAIOR,
    AST_MAIOR_IGUAL
} TipoNoAST;

typedef union
{
    long i;
    double r;
} ValorAST;

typedef struct NoAST
{
    TipoNoAST tipo;
    int tipo_c;
    Simbolo *simbolo;
    char *texto;
    ValorAST valor;
    int linha;
    int coluna;

    struct NoAST *esq;
    struct NoAST *dir;
    struct NoAST *terceiro;
    struct NoAST *proximo;
} NoAST;

NoAST *ast_criar_numero_int(int tipo_c, long valor, int linha, int coluna);
NoAST *ast_criar_numero_real(int tipo_c, double valor, int linha, int coluna);
NoAST *ast_criar_identificador(const char *nome, int linha, int coluna);
NoAST *ast_criar_operacao(TipoNoAST tipo, NoAST *esq, NoAST *dir, int linha, int coluna);
NoAST *ast_criar_menos_unario(NoAST *operando, int linha, int coluna);
NoAST *ast_criar_declaracao(int tipo_c, const char *nome, NoAST *inicializacao, int linha, int coluna);
NoAST *ast_criar_atribuicao(const char *nome, NoAST *expressao, int linha, int coluna);
NoAST *ast_criar_expressao(NoAST *expressao, int linha, int coluna);
NoAST *ast_criar_bloco(NoAST *instrucoes);
NoAST *ast_criar_if(NoAST *condicao, NoAST *bloco_entao, NoAST *bloco_senao, int linha, int coluna);
NoAST *ast_criar_programa(NoAST *instrucoes);
NoAST *ast_anexar(NoAST *lista, NoAST *no);

void ast_imprimir(const NoAST *raiz, int nivel);
void ast_liberar(NoAST *raiz);

#endif