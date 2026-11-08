#ifndef EXECUCAO_H
#define EXECUCAO_H

#include "ast.h"

/* Executa o programa representado pela AST.
 * Retorna 1 em caso de sucesso e 0 em caso de erro.
 */
int execucao_executar(NoAST *raiz);

#endif