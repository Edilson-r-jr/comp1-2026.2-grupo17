/* FGA0003 - Compiladores 1
 * Curso de Engenharia de Software
 * Universidade de Brasília (UnB)
 * Semana 10 - prática de geração de código final
 * gerador_js.h - Geração de código JavaScript a partir da AST
 *
 * Última fase do compilador: traduz o programa C para JavaScript.
 * O código é gerado a partir da AST (validada pela semântica), e não do
 * TAC, porque o JavaScript não tem goto: if/else e while viram as mesmas
 * estruturas no JS.
 *
 * O C é estaticamente tipado e o JavaScript guarda todo número como um
 * double de 64 bits. Por isso o gerador usa o tipo de cada nó (tipo_c,
 * anotado pela semântica) para simular o comportamento do C; as regras
 * estão no README, seção "Geração de JavaScript".
 * Issue #51: declarações, atribuições e expressões.
 * Issue #52: controle de fluxo (blocos, if/else, while).
 * Controle de fluxo (blocos, if/else, while) fica para a issue #52.
 */
#ifndef GERADOR_JS_H
#define GERADOR_JS_H

#include <stdio.h>
#include "ast.h"

/* Gera o programa JavaScript (raiz = AST_PROGRAMA) em 'saida'.
 * Deve ser chamada só para programas sem erros. */
void gerador_js_gerar(const NoAST *raiz, FILE *saida);

#endif /* GERADOR_JS_H */
