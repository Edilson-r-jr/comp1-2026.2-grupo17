# Relatório Semanal - Semana 10

**Projeto:** Compilador (comp1-2026.2-grupo17)
**Disciplina:** Compiladores 1

> **Organização das Semanas 9 e 10:** as duas semanas foram conduzidas como uma única sprint de fechamento. A otimização, a geração de JavaScript, o runtime de saída e os testes ponta a ponta formaram uma sequência integrada. Este relatório complementa a Semana 9 com a conclusão da geração de código final e a preparação da entrega.

## Objetivos da Semana

- Traduzir a AST validada para JavaScript.
- Suportar declarações, atribuições, expressões, blocos, condicionais e laços.
- Preservar o comportamento dos tipos numéricos de C no JavaScript.
- Implementar `printf` para observar a saída dos programas.
- Automatizar testes ponta a ponta comparando Node.js e C.
- Atualizar a documentação e organizar a entrega final.

## Issue #51 — Gerador de JavaScript: declarações, atribuições e expressões

- **Responsável:** Marcus Rezende.
- **Entrega:** [PR #55 — gerador de JavaScript](https://github.com/Edilson-r-jr/comp1-2026.2-grupo17/pull/55).
- **Merge:** 24/11/2026, commit `839fc2b`.
- **Atividades:** criação de `src/gerador_js.c` e `src/gerador_js.h`; geração de declarações, atribuições e expressões; inicialização de variáveis sem valor explícito; conversões numéricas baseadas em `tipo_c`; opção `--js`.
- **Decisão técnica:** a saída é gerada diretamente da AST, já validada pela semântica, e não do TAC. O TAC representa desvios com `goto`, recurso inexistente em JavaScript.
- **Escopos:** variáveis de blocos internos usam nomes como `x$1`, evitando colisões.

## Issue #52 — Gerador de JavaScript: if/else, while e blocos

- **Responsável:** Kael-web7.
- **Entrega:** [PR #56 — estruturas de controle em JavaScript](https://github.com/Edilson-r-jr/comp1-2026.2-grupo17/pull/56).
- **Merge:** 25/11/2026, commit `5cc4d5c`.
- **Atividades:** geração de blocos, `if`, `if/else` e `while`; indentação de quatro espaços; condições relacionais emitidas diretamente em estruturas de controle; suporte a estruturas aninhadas e sombreamento.
- **Decisão técnica:** comparações em condições podem ser emitidas como booleanos JavaScript; quando usadas como expressões numéricas do C, continuam convertidas para `1` ou `0`.

## Issue #53 — printf e testes do JavaScript gerado no Node.js

- **Responsável:** Edilson Júnior.
- **Entrega:** [PR #58 — printf, runtime JS e testes Node.js](https://github.com/Edilson-r-jr/comp1-2026.2-grupo17/pull/58).
- **Merge:** 27/11/2026, commit `3c6f650`.
- **Atividades:** reconhecimento de strings e escapes; aceitação de `#include <stdio.h>` e `#include <stdbool.h>`; nó `AST_PRINTF`; validação semântica de especificadores e argumentos; runtime emitido somente quando necessário; suporte a `%d`, `%i`, `%ld`, `%li`, `%c`, `%f`, `%lf` e `%%`.
- **Testes ponta a ponta:** programas de `tests/entradas_js/` são compilados com `--js`, executados pelo Node.js e comparados com respostas de referência geradas a partir dos mesmos programas em C com GCC. Cada caso também roda com `--otimizar`.
- **Correções de integração:** compatibilidade de `printf`, diretivas com finais de linha CRLF e validação da execução ao gerar respostas JavaScript.

## Issue #54 — Documentação final e release v1.0

- **Responsável:** Pedro Henrique.
- **Situação:** documentação e organização da entrega em andamento na branch `docs/issue-54`.
- **Escopo:** revisão do README, relatórios das Semanas 9 e 10, exemplos de demonstração e preparação dos artefatos da release `v1.0`.

## Arquitetura e decisões técnicas consolidadas

- Fluxo: scanner → parser/AST → análise semântica e tabela de símbolos → otimização opcional → TAC ou JavaScript.
- O TAC usa temporárias, rótulos e saltos; o JavaScript é emitido diretamente da AST para preservar estruturas de controle.
- A semântica valida os argumentos de `printf` antes da geração.
- O runtime de `printf` é incluído apenas quando o programa usa essa instrução.
- A otimização é opcional e compartilhada pelas saídas de TAC e JavaScript.

## Dificuldades encontradas e soluções

- **Diferenças numéricas entre C e JavaScript:** conversões específicas por tipo, incluindo `Math.fround`, `Math.imul`, `| 0` e `Math.trunc`.
- **Controle de fluxo no código final:** percurso da AST para emitir diretamente `if`, `else` e `while`, sem converter os saltos do TAC.
- **Sombreamento:** nomenclatura com `$` padronizada entre TAC e JavaScript.
- **Ausência de saída observável:** runtime de `printf` e comparação com GCC/Node.js.
- **Diferenças entre ambientes:** suporte a finais de linha CRLF nos includes aceitos.

## Testes e validação

A integração final reúne as baterias de compilador/AST, scanner, TAC, otimização e JavaScript no comando `make -C src test`. A bateria de JavaScript requer Node.js; quando disponível, executa os programas gerados e compara suas saídas com referências em C.

Os relatórios de PR registram, para a etapa de otimização, 70/70 testes aprovados, verificações de memória e comparação dos resultados otimizados e não otimizados. As issues de geração de JavaScript acrescentaram testes de estruturas aninhadas, escopos, tipos e saída de `printf`.

## Resultado da sprint e pendências de entrega

As funcionalidades centrais planejadas para a tradução C → JavaScript foram integradas à branch principal: otimização opcional, declarações, atribuições, expressões, blocos, `if/else`, `while` e `printf`.

A release `v1.0` ainda depende da compilação e validação dos executáveis Linux e Windows, da geração dos hashes SHA-256 e da publicação dos artefatos e notas no GitHub. Esses passos devem ser concluídos nos ambientes de build correspondentes antes de marcar a entrega como publicada.
