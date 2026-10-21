/* Teste do módulo da AST (src/ast.c), sem depender do parser.
 * Monta árvores à mão, imprime com ast_imprimir e libera com ast_liberar.
 * A saída é comparada com tests/ast/esperado.txt por run_ast_tests.sh. */
#include <stdio.h>
#include "ast.h"

int main(void)
{
    /* Critério de aceite da issue #23: a + b * 2 */
    NoAST *expr = ast_criar_operacao(AST_SOMA,
        ast_criar_identificador("a", 1, 1),
        ast_criar_operacao(AST_MULTIPLICACAO,
            ast_criar_identificador("b", 1, 5),
            ast_criar_numero_int(TIPO_INT, 2, 1, 9),
            1, 7),
        1, 3);
    ast_imprimir(expr, 0);
    ast_liberar(expr);

    printf("---\n");

    /* Programa com todos os tipos de nó:
     *   int x = 5 / 2;  double y = 5.0;  float f = 2.5;
     *   long l = 10L;   char c = 'a';    char n = '\n';
     *   x = -x;         x * 3; */
    NoAST *lista = NULL;
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_INT, "x",
        ast_criar_operacao(AST_DIVISAO,
            ast_criar_numero_int(TIPO_INT, 5, 1, 9),
            ast_criar_numero_int(TIPO_INT, 2, 1, 13), 1, 11),
        1, 5));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_DOUBLE, "y",
        ast_criar_numero_real(TIPO_DOUBLE, 5.0, 2, 12), 2, 8));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_FLOAT, "f",
        ast_criar_numero_real(TIPO_FLOAT, 2.5, 3, 11), 3, 7));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_LONG, "l",
        ast_criar_numero_int(TIPO_LONG, 10, 4, 10), 4, 6));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_CHAR, "c",
        ast_criar_numero_int(TIPO_CHAR, 'a', 5, 10), 5, 6));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_CHAR, "n",
        ast_criar_numero_int(TIPO_CHAR, '\n', 6, 10), 6, 6));
    lista = ast_anexar(lista, ast_criar_declaracao(TIPO_INT, "z", NULL, 7, 5));
    lista = ast_anexar(lista, ast_criar_atribuicao("x",
        ast_criar_menos_unario(ast_criar_identificador("x", 8, 6), 8, 5), 8, 1));
    lista = ast_anexar(lista, ast_criar_expressao(
        ast_criar_operacao(AST_MULTIPLICACAO,
            ast_criar_identificador("x", 9, 1),
            ast_criar_numero_int(TIPO_INT, 3, 9, 5), 9, 3),
        9, 1));

    NoAST *programa = ast_criar_programa(lista);
    ast_imprimir(programa, 0);
    ast_liberar(programa);

    /* Casos de borda: árvore vazia */
    ast_imprimir(NULL, 0);
    ast_liberar(NULL);

    return 0;
}
