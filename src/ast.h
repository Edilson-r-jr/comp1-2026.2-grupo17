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
    /* Estrutura */
    AST_PROGRAMA,      /* esq = primeira instrução (lista via 'proximo') */
    AST_DECLARACAO,    /* texto = nome, tipo_c = tipo declarado, esq = inicialização (ou NULL) */
    AST_ATRIBUICAO,    /* texto = nome da variável, esq = expressão */
    AST_EXPRESSAO,     /* instrução-expressão (ex.: "x * 3;"), esq = expressão */

    /* Folhas */
    AST_NUMERO,        /* literal; tipo_c diz se o valor está em .i ou .r */
    AST_IDENTIFICADOR, /* uso de variável, texto = nome */

    /* Operações */
    AST_SOMA,
    AST_SUBTRACAO,
    AST_MULTIPLICACAO,
    AST_DIVISAO,
    AST_MENOS_UNARIO   /* esq = operando */
} TipoNoAST;

/* Valor de um literal. O campo válido depende de tipo_c:
 * char/int/long usam .i, float/double usam .r.
 * Guardar inteiros como long (e não double) preserva a diferença entre
 * 5 e 5.0, que muda o resultado da divisão em C, e a precisão de long. */
typedef union
{
    long i;
    double r;
} ValorAST;

typedef struct NoAST
{
    TipoNoAST tipo;        /* categoria do nó */
    int tipo_c;            /* um dos TIPO_* de tabela.h, ou TIPO_INEXISTENTE
                              enquanto a análise semântica não anotar o nó */
    char *texto;           /* nome de variável (cópia própria) */
    ValorAST valor;        /* só para AST_NUMERO */
    int linha;
    int coluna;
    struct NoAST *esq;     /* filho esquerdo / único filho */
    struct NoAST *dir;     /* filho direito */
    struct NoAST *proximo; /* próxima instrução da mesma lista */
} NoAST;

/* Literais */
NoAST *ast_criar_numero_int(int tipo_c, long valor, int linha, int coluna);
NoAST *ast_criar_numero_real(int tipo_c, double valor, int linha, int coluna);

/* Expressões */
NoAST *ast_criar_identificador(const char *nome, int linha, int coluna);
NoAST *ast_criar_operacao(TipoNoAST tipo, NoAST *esq, NoAST *dir, int linha, int coluna);
NoAST *ast_criar_menos_unario(NoAST *operando, int linha, int coluna);

/* Instruções */
NoAST *ast_criar_declaracao(int tipo_c, const char *nome, NoAST *inicializacao,
                            int linha, int coluna);
NoAST *ast_criar_atribuicao(const char *nome, NoAST *expressao, int linha, int coluna);
NoAST *ast_criar_expressao(NoAST *expressao, int linha, int coluna);
NoAST *ast_criar_programa(NoAST *instrucoes);

/* Coloca 'no' no fim da lista iniciada em 'lista' e devolve o início.
 * Pensado para a regra  programa: programa instrucao  do Bison. */
NoAST *ast_anexar(NoAST *lista, NoAST *no);

void ast_imprimir(const NoAST *raiz, int nivel);
void ast_liberar(NoAST *raiz);

#endif /* AST_H */
