/* FGA0003 - Compiladores 1
 * tabela.h - Tabela de Símbolos
 *
 * Guarda as variáveis declaradas no programa (nome + tipo + valor atual)
 * em uma lista encadeada. É um arquivo "header-only": as funções são
 * static inline, então pode ser incluído em mais de uma unidade de
 * tradução sem gerar símbolos duplicados nem warnings de função não usada.
 */
#ifndef TABELA_H
#define TABELA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tipos suportados pela linguagem.
 * A ordem importa: quanto maior o valor, mais "largo" é o tipo.
 * Isso é usado na promoção de tipos em expressões mistas
 * (ex.: long + double -> double), como no C. */
typedef enum {
    TIPO_CHAR = 0,
    TIPO_INT,
    TIPO_LONG,
    TIPO_FLOAT,
    TIPO_DOUBLE
} Tipo;

/* Retorno de verificar_tipo() quando a variável não existe. */
#define TIPO_INEXISTENTE (-1)

/* Uma entrada da tabela. */
typedef struct Simbolo {
    char *nome;             /* nome da variável (cópia própria) */
    int tipo;               /* um dos valores de Tipo */
    long valor_int;         /* valor quando o tipo é inteiro (char/int/long) */
    double valor_real;      /* valor quando o tipo é real (float/double) */
    struct Simbolo *prox;   /* próximo da lista */
} Simbolo;

/* Cabeça da lista encadeada (global, vive durante toda a execução). */
static Simbolo *tabela_simbolos = NULL;

/* Quantidade de erros semânticos encontrados. Se for > 0 ao final,
 * o programa termina com código de saída diferente de zero. */
static int erros_semanticos = 0;

/* Linha atual, mantida pelo Flex (%option yylineno). */
extern int yylineno;

static inline const char *nome_tipo(int tipo) {
    switch (tipo) {
        case TIPO_CHAR:   return "char";
        case TIPO_INT:    return "int";
        case TIPO_LONG:   return "long";
        case TIPO_FLOAT:  return "float";
        case TIPO_DOUBLE: return "double";
        default:          return "desconhecido";
    }
}

static inline int tipo_eh_real(int tipo) {
    return tipo == TIPO_FLOAT || tipo == TIPO_DOUBLE;
}

static inline void erro_semantico(const char *fmt, const char *nome) {
    fprintf(stderr, "Erro Semantico (linha %d): ", yylineno);
    fprintf(stderr, fmt, nome);
    fputc('\n', stderr);
    erros_semanticos++;
}

/* Procura uma variável pelo nome. Retorna NULL se não existir. */
static inline Simbolo *buscar_variavel(const char *nome) {
    for (Simbolo *s = tabela_simbolos; s != NULL; s = s->prox) {
        if (strcmp(s->nome, nome) == 0) {
            return s;
        }
    }
    return NULL;
}

/* Registra uma nova variável. Redeclaração é erro semântico. */
static inline void inserir_variavel(char *nome, int tipo) {
    if (buscar_variavel(nome) != NULL) {
        erro_semantico("variavel '%s' ja foi declarada", nome);
        return;
    }

    Simbolo *novo = malloc(sizeof(Simbolo));
    if (novo == NULL) {
        perror("malloc");
        exit(1);
    }
    novo->nome = malloc(strlen(nome) + 1);
    if (novo->nome == NULL) {
        perror("malloc");
        exit(1);
    }
    strcpy(novo->nome, nome);
    novo->tipo = tipo;
    novo->valor_int = 0;
    novo->valor_real = 0.0;

    /* insere no início da lista: O(1) */
    novo->prox = tabela_simbolos;
    tabela_simbolos = novo;
}

/* Retorna o tipo de uma variável já declarada,
 * ou TIPO_INEXISTENTE se ela não estiver na tabela. */
static inline int verificar_tipo(char *nome) {
    Simbolo *s = buscar_variavel(nome);
    return s ? s->tipo : TIPO_INEXISTENTE;
}

/* Libera toda a tabela (chamada ao final do main). */
static inline void liberar_tabela(void) {
    Simbolo *s = tabela_simbolos;
    while (s != NULL) {
        Simbolo *prox = s->prox;
        free(s->nome);
        free(s);
        s = prox;
    }
    tabela_simbolos = NULL;
}

#endif /* TABELA_H */
