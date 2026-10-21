/* FGA0003 - Compiladores 1
 * tabela.h - Tabela de Símbolos (interface)
 *
 * Guarda os nomes declarados no programa em uma lista encadeada.
 * Cada símbolo registra nome, categoria, tipo, escopo e a posição
 * (linha/coluna) da declaração.
 *
 * Nesta etapa só existe o escopo global (ESCOPO_GLOBAL), mas todas as
 * operações já recebem o escopo como parâmetro para que escopos aninhados
 * (blocos, funções) possam ser adicionados sem mudar a interface.
 *
 * A implementação fica em tabela.c.
 */
#ifndef TABELA_H
#define TABELA_H

#include <stdio.h>

/* Tipos suportados pela linguagem.
 * A ordem importa: quanto maior o valor, mais "largo" é o tipo.
 * Isso é usado na promoção de tipos em expressões mistas
 * (ex.: long + double -> double), como no C.
 * TIPO_VOID fica no final porque nunca participa de expressões. */
typedef enum
{
    TIPO_CHAR = 0,
    TIPO_INT,
    TIPO_LONG,
    TIPO_FLOAT,
    TIPO_DOUBLE,
    TIPO_VOID
} Tipo;

/* O que o nome representa no programa. */
typedef enum
{
    CAT_VARIAVEL = 0,
    CAT_FUNCAO,
    CAT_PARAMETRO
} Categoria;

/* Escopo global. Escopos aninhados terão números maiores. */
#define ESCOPO_GLOBAL 0

/* Uma entrada da tabela. */
typedef struct Simbolo
{
    char *nome;           /* nome (cópia própria) */
    Categoria categoria;  /* variável, função, parâmetro */
    Tipo tipo;            /* tipo declarado */
    int escopo;           /* ESCOPO_GLOBAL por enquanto */
    int linha;            /* posição da declaração */
    int coluna;

    /* Valor atual da variável. Usado enquanto o parser ainda avalia as
     * expressões diretamente; deixa de ser necessário quando a AST
     * assumir esse papel. */
    long valor_int;       /* char/int/long */
    double valor_real;    /* float/double */

    struct Simbolo *prox; /* próximo da lista */
} Simbolo;

/* ---------- Operações da tabela ---------- */

/* Insere um novo símbolo no final da tabela.
 * Retorna o símbolo criado, ou NULL se o nome já existir no mesmo escopo
 * (nesse caso o erro semântico de redeclaração já foi reportado). */
Simbolo *tabela_inserir(const char *nome, Categoria categoria, Tipo tipo,
                        int escopo, int linha, int coluna);

/* Procura um nome no escopo informado e, se não achar, no escopo global.
 * Retorna NULL se o nome não estiver declarado. */
Simbolo *tabela_buscar(const char *nome, int escopo);

/* Imprime a tabela, em ordem de declaração, no arquivo informado. */
void tabela_imprimir(FILE *saida);

/* Libera todos os símbolos. Chamada uma vez, ao final do main. */
void tabela_liberar(void);

/* ---------- Auxiliares ---------- */

const char *nome_tipo(int tipo);
const char *nome_categoria(int categoria);
int tipo_eh_real(int tipo);

/* ---------- Erros semânticos ---------- */

/* Quantidade de erros semânticos encontrados. Se for > 0 ao final,
 * o programa termina com código de saída diferente de zero. */
extern int erros_semanticos;

/* Posição (linha/coluna) usada nas mensagens de erro semântico.
 * O parser atualiza com definir_posicao(@N) antes de chamar a tabela. */
void definir_posicao(int linha, int coluna);

/* Mensagem no mesmo formato dos erros léxicos e sintáticos:
 *   Erro Semântico [Linha X, Col Y]: <mensagem>
 * Aceita argumentos no estilo printf. */
void erro_semantico(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));

#endif /* TABELA_H */