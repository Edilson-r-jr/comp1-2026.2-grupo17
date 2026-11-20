/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 8 - prática de geração de código intermediário
 * tac.h - Geração de código de três endereços (TAC) a partir da AST
 *
 * Roda depois da análise semântica, só para programas sem erros: a AST já
 * foi construída pelo parser e validada, então cada nó pode ser traduzido
 * sem novas verificações.
 *
 * Cada instrução TAC tem no máximo um operador e três endereços:
 *   t0 = b * c        (operação binária, resultado numa temporária)
 *   t1 = - a          (operação unária)
 *   x = t0            (cópia: atribuição sem temporária extra)
 *
 * Issue #41: expressões, atribuições e declarações.
 * Controle de fluxo (if/else, while) fica para as issues #42 e #44.
 */
#ifndef TAC_H
#define TAC_H

#include <stdio.h>
#include "ast.h"

/* Gera o TAC do programa inteiro (raiz = AST_PROGRAMA) em 'saida'.
 * As temporárias recomeçam de t0 a cada chamada. */
void tac_gerar(const NoAST *raiz, FILE *saida);

#endif /* TAC_H */
