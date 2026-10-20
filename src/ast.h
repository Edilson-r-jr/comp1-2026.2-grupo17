#ifndef AST_H
#define AST_H

typedef enum {
    AST_PROGRAMA,
    AST_DECLARACAO,
    AST_NUMERO,
    AST_IDENTIFICADOR,
    AST_SOMA,
    AST_SUBTRACAO,
    AST_MULTIPLICACAO,
    AST_DIVISAO
} TipoNoAST;

typedef struct NoAST {
    TipoNoAST tipo;
    char* texto;           
    double valor;          
    int linha;
    int coluna;
    struct NoAST* esq;     
    struct NoAST* dir;     
    struct NoAST* proximo; 
} NoAST;

NoAST* ast_criar_numero(double valor, int linha);
NoAST* ast_criar_identificador(const char* nome, int linha);
NoAST* ast_criar_operacao(TipoNoAST tipo, NoAST* esq, NoAST* dir, int linha);
NoAST* ast_criar_declaracao(const char* nome, NoAST* inicializacao, int linha);

void ast_imprimir(NoAST* raiz, int nivel);
void ast_liberar(NoAST* raiz);

#endif