/* FGA0003 - Compiladores 1
 * semantica.h - Análise semântica sobre a AST
 *
 * Roda depois do parsing: o parser só constrói a árvore, e este módulo a
 * percorre para registrar as declarações na tabela de símbolos e resolver
 * cada uso de nome. Os erros seguem o padrão do projeto:
 *   Erro Semântico [Linha X, Col Y]: <causa>
 *
 * Nesta etapa (issue #29) são verificados:
 *   - redeclaração no mesmo escopo;
 *   - uso de variável não declarada (inclusive uso antes da declaração);
 *   - divisão por um literal zero.
 * Issue #30: cada expressão recebe seu tipo em no->tipo_c e atribuições
 * de real para inteiro são rejeitadas (regras no README).
 */
#ifndef SEMANTICA_H
#define SEMANTICA_H

#include "ast.h"

/* Analisa o programa inteiro a partir da raiz (AST_PROGRAMA).
 * Retorna a quantidade de erros semânticos encontrados nesta análise. */
int semantica_analisar(NoAST *raiz);

#endif /* SEMANTICA_H */
