#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Função auxiliar para criar um nó base
static NoAST* criar_no_base(TipoNoAST tipo, int linha) {
    NoAST* no = (NoAST*)calloc(1, sizeof(NoAST));
    if (!no) {
        fprintf(stderr, "Erro: Falha na alocação de memória para a AST.\n");
        exit(1);
    }
    no->tipo = tipo;
    no->linha = linha;
    return no;
}

NoAST* ast_criar_numero(double valor, int linha) {
    NoAST* no = criar_no_base(AST_NUMERO, linha);
    no->valor = valor;
    return no;
}

NoAST* ast_criar_identificador(const char* nome, int linha) {
    NoAST* no = criar_no_base(AST_IDENTIFICADOR, linha);
    no->texto = strdup(nome);
    return no;
}

NoAST* ast_criar_operacao(TipoNoAST tipo, NoAST* esq, NoAST* dir, int linha) {
    NoAST* no = criar_no_base(tipo, linha);
    no->esq = esq;
    no->dir = dir;
    return no;
}

NoAST* ast_criar_declaracao(const char* nome, NoAST* inicializacao, int linha) {
    NoAST* no = criar_no_base(AST_DECLARACAO, linha);
    no->texto = strdup(nome);
    no->esq = inicializacao;
    return no;
}

// Retorna o nome da string baseada no Enum
static const char* obter_nome_tipo(TipoNoAST tipo) {
    switch(tipo) {
        case AST_PROGRAMA: return "AST_PROGRAMA";
        case AST_DECLARACAO: return "AST_DECLARACAO";
        case AST_NUMERO: return "AST_NUMERO";
        case AST_IDENTIFICADOR: return "AST_IDENTIFICADOR";
        case AST_SOMA: return "AST_SOMA";
        case AST_SUBTRACAO: return "AST_SUBTRACAO";
        case AST_MULTIPLICACAO: return "AST_MULTIPLICACAO";
        case AST_DIVISAO: return "AST_DIVISAO";
        default: return "AST_DESCONHECIDO";
    }
}

void ast_imprimir(NoAST* raiz, int nivel) {
    if (!raiz) return;

    for (int i = 0; i < nivel; i++) {
        printf("  "); // 2 espaços por nível de indentação
    }

    printf("%s", obter_nome_tipo(raiz->tipo));

    if (raiz->tipo == AST_IDENTIFICADOR || raiz->tipo == AST_DECLARACAO) {
        printf(" (%s)", raiz->texto);
    } else if (raiz->tipo == AST_NUMERO) {
        // Formata para evitar zeros desnecessários em inteiros
        if (raiz->valor == (int)raiz->valor) {
            printf(" (%d)", (int)raiz->valor);
        } else {
            printf(" (%g)", raiz->valor);
        }
    }
    printf("\n");

    ast_imprimir(raiz->esq, nivel + 1);
    ast_imprimir(raiz->dir, nivel + 1);
    ast_imprimir(raiz->proximo, nivel); // O próximo comando fica no mesmo nível
}

void ast_liberar(NoAST* raiz) {
    if (!raiz) return;

    ast_liberar(raiz->esq);
    ast_liberar(raiz->dir);
    ast_liberar(raiz->proximo);

    if (raiz->texto) {
        free(raiz->texto);
    }
    free(raiz);
}