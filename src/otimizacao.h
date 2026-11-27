/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 9 - prática de otimização de código
 * otimizacao.h - Constant folding sobre a AST
 *
 * Calcula em tempo de compilação as operações cujos operandos são todos
 * constantes e troca o nó da operação por um AST_NUMERO com o resultado.
 * Também simplifica x + 0, x - 0, x * 1 e x / 1 para x.
 * Roda depois da análise semântica (os nós já têm tipo_c) e antes do TAC
 * e do JavaScript, então as duas saídas se beneficiam.
 */
#ifndef OTIMIZACAO_H
#define OTIMIZACAO_H

#include "ast.h"

/* Aplica o constant folding em todas as instruções do programa, inclusive
 * dentro de blocos, if/else e while. Só deve ser chamada para programas
 * sem erros (a AST precisa estar completa e anotada pela semântica). */
void otimizacao_aplicar(NoAST *raiz);

#endif /* OTIMIZACAO_H */
