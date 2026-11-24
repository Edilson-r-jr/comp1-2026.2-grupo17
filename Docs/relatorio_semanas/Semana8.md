# Relatório Semanal - Semana 8

**Projeto:** Compilador (comp1-2026.2-grupo17)

**Disciplina:** Compiladores

> Situação registrada em 24/11/2026, após o merge do PR #47.

## Objetivos da Semana

- Iniciar a geração de código intermediário: o código de três endereços (TAC), produzido a partir da AST já validada pela análise semântica.
- Gerar TAC para expressões e atribuições.
- Gerar TAC para `if`/`else`, com rótulos e desvios.
- Acrescentar o laço `while`.
- Automatizar os testes de TAC.
- Registrar as Semanas 7 e 8 nos relatórios.

## Issues da Semana 8

### 1. Issue #41: `Geração de TAC para expressões e atribuições`

- **Status:** Concluída
- **Responsável:** Marcus Rezende
- **Entrega:** PR #46 (commit `86eb095`)
- **Principais arquivos:** `src/tac.c`, `src/tac.h`, `src/parser.y`, `src/makefile`, `README.md`
- **Atividades Desenvolvidas:**
    - Criação do módulo `tac.c`, que percorre a AST e emite uma instrução por operação.
    - Nova opção `./parser --tac` para imprimir o código intermediário.
    - Nova seção "Código intermediário (TAC)" no `README.md`, com um exemplo de entrada e saída:

      ```text
      y = (a + b) * (c - 2);   →   t0 = a + b
                                   t1 = c - 2
                                   t2 = t0 * t1
                                   y = t2
      ```
- **Decisões Técnicas:**
    - Cada instrução tem no máximo um operador. Os resultados intermediários ficam em temporárias `t0`, `t1`, ..., que recomeçam do zero a cada programa.
    - A atribuição copia o resultado direto para a variável (`y = t2`), sem temporária extra.
    - Declaração sem inicialização (`int y;`) não gera instrução.
    - O TAC só é gerado para programas sem erros léxicos, sintáticos ou semânticos, então o gerador não repete verificações.

### 2. Issue #42: `Gerar TAC para if/else com rótulos e saltos`

- **Status:** Concluída (merge em 24/11), com uma correção registrada para a próxima sprint
- **Responsável:** Pedro Henrique
- **Revisão:** Maria Laura
- **Entrega:** PR #47 (commit `8d8636b`)
- **Principais arquivos:** `src/tac.c`, `src/makefile`, `tests/run_tac_tests.sh`, `tests/entradas_tac/`, `tests/respostas_tac/`
- **Atividades Desenvolvidas:**
    - Tradução do `if` com desvio condicional (`ifFalse ... goto`) e do `else` com desvio incondicional (`goto`), usando rótulos `L0`, `L1`, ...:

      ```text
      if (a < b) { x = 1; }   →   t0 = a < b
      else { x = 2; }             ifFalse t0 goto L0
                                  x = 1
                                  goto L1
                                  L0:
                                  x = 2
                                  L1:
      ```
    - Variáveis declaradas dentro de blocos recebem o número do escopo no nome (`x_1`), para que uma variável sombreada não sobrescreva a de fora.
    - Criação de `tests/run_tac_tests.sh`, incluído no `make test`, com cinco casos: `if` sem `else`, `if`/`else`, `if` aninhado, condição com identificador e escopos.
    - A suíte completa passa: 35 testes do compilador, AST, scanner e 5 testes de TAC, sem vazamentos de memória no `valgrind`.
- **Pendência encontrada na revisão:**
    - O sufixo `_N` pode colidir com um nome declarado pelo usuário. Em `int x_1 = 5; { int x = 2; x_1 = x; }`, a variável local também vira `x_1` e sobrescreve a global. A correção, que é usar um separador inválido em identificadores C (como `x$1`) e acrescentar esse caso aos testes, ficou para a próxima sprint.

### 3. Issue #44: `Laço while`

- **Status:** Não iniciada
- **Situação:** o token `WHILE` existe no scanner, mas a gramática ainda não tem regra para o laço, e o TAC não o gera.

### 4. Issue #45: `Relatórios das Semanas 7 e 8`

- **Status:** Concluída com este documento
- **Responsável:** Maria Laura
- **Principais arquivos:** `Docs/relatorio_semanas/Semana7.md`, `Docs/relatorio_semanas/Semana8.md`

### Entregas da Semana 7 integradas neste período

- **Issue #31 (PR #40, merge em 19/11):** diagnósticos semânticos acumulados e AST anotada. Está descrita no `Semana7.md`, porque pertence ao milestone da Semana 7.

## Dificuldades Encontradas e Soluções Adotadas

* **Colisão de nomes no TAC de blocos:** a revisão do PR #47 encontrou o caso de `x_1` descrito na Issue #42. Como o caso é raro e a correção é pequena, o grupo decidiu integrar o PR e corrigir na próxima sprint.
* **Documentação do TAC desatualizada:** depois do PR #47, o `README.md` e o comentário do `tac.h` ainda dizem que o `if`/`else` não gera TAC.
* **PR #47 aberto e integrado no último dia:** a revisão aconteceu no mesmo dia do merge, o que deixou pouco tempo para os ajustes.
* **`while` não iniciado:** ele depende de uma regra nova na gramática, de um nó na AST e da análise semântica, antes de chegar ao TAC. O padrão de rótulos e desvios do PR #47 serve de base para o laço.

## Próximos Passos (Semana 9)

- Corrigir a colisão de nomes das variáveis de blocos no TAC e incluir o caso nos testes.
- Atualizar o `README.md` e o `tac.h` com o TAC de `if`/`else`.
- Implementar o `while` na gramática, na AST, na semântica e no TAC (Issue #44), com testes no `run_tac_tests.sh`.
- Iniciar a geração de JavaScript a partir da AST anotada.
