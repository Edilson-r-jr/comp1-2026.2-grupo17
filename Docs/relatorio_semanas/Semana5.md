# Relatório Semanal - Semana 5

**Projeto:** Compilador (comp1-2026.2-grupo17)

**Disciplina:** Compiladores

## Objetivos da Semana

- Integrar novos tokens léxicos com a gramática no analisador sintático.
- Implementar regras de derivação para estruturas de decisão (`if`, `else`) e blocos de comandos.
- Padronizar o tratamento de erros léxicos e sintáticos com indicação de linha e coluna.
- Consolidar a documentação de escopo e garantir cobertura por testes automatizados.

## Issues Fechadas na Semana 5

### 1. Issue #14: `Especificação e delimitação do escopo da linguagem`

- **Status:** Concluída
- **Entrega:** commit `6e3404a`
- **Principais arquivos:** `README.md`
- **Atividades Desenvolvidas:**
    - Criação da seção "Escopo" no `README.md`, com a análise das diferenças entre C e JavaScript que orientou a decisão do grupo.
    - Definição dos requisitos dentro do escopo: tipos `char`, `int`, `long`, `float` e `double`, estruturas de controle, funções e geração de código JavaScript, com o status de cada um.
    - Definição dos recursos fora do escopo e do motivo de cada exclusão, como ponteiros, `malloc`/`free`, `struct`/`union` e `goto`.

### 2. Issue #15: `Refinamento e categorização do sistema de mensagens de erro`

- **Status:** Concluída
- **Entrega:** PR #19 (commits `02ab7f6` e `524b3c9`)
- **Principais arquivos:** `src/scanner.l`, `src/parser.y`, `tests/entradas_erro/comentario_multilinha.txt`, `tests/respostas_erro/comentario_multilinha.txt`
- **Atividades Desenvolvidas:**
    - Diferenciação dos erros por etapa do pipeline, com os rótulos `Erro Léxico`, `Erro Sintático` e `Erro Semântico`.
    - Inclusão de linha e coluna nas mensagens, com `%option yylineno` no scanner e `%locations` no parser.
    - Tradução das mensagens sintáticas para o português, no lugar do `syntax error` padrão do Bison.
    - Contagem correta das quebras de linha dentro de comentários de bloco.
    - Mensagem própria para divisão por zero, que antes era rotulada como erro sintático.
    - Os erros de variável não declarada e de redeclaração ficaram de fora do PR #19, porque a gramática ainda não tinha declarações. Eles foram entregues no PR #21, no mesmo formato de mensagem.

### 3. Issue #16: `Adicionar suporte a estruturas de decisão (if/else) no scanner.l`

- **Status:** Parcial (fechada em 14/10 com um critério de aceitação pendente)
- **Entrega:** commit `63c99ee`, integrado pelo PR #19
- **Principais arquivos:** `tests/test_scanner.c`, `tests/run_scanner_tests.sh`, `tests/entradas_scanner/controle_condicional.txt`
- **Atividades Desenvolvidas:**
    - Os tokens `IF` e `ELSE` já existiam no `scanner.l` antes desta semana. O trabalho da issue foi validar esse reconhecimento.
    - Criação de um teste que verifica se `if` e `else` geram os tokens corretos e se `ifelse_var` continua sendo um identificador.
    - **Pendente:** o critério "rodar a suíte de testes do scanner com blocos `if` e `else`" ficou desmarcado na issue. Veja a seção de dificuldades.

### 4. Issue #17: `Resumo de atividades da Semana 04`

- **Status:** Concluída
- **Entrega:** PR #22
- **Principais arquivos:** `Docs/relatorio_semanas/Semana4.md`
- **Atividades Desenvolvidas:**
    - Criação do relatório da Semana 4, com as issues concluídas e os próximos passos.

### 5. Issue #18: `Criação de suíte de testes para atribuição e estruturas condicionais (if/else)`

- **Status:** Parcial (fechada em 14/10, sem PR vinculado)
- **Entrega:** commit `8dc1993`, integrado pelo PR #21
- **Principais arquivos:** `tests/entradas/`, `tests/entradas_erro/`, `tests/respostas_erro/`
- **Atividades Desenvolvidas:**
    - Atualização dos testes existentes para a nova gramática, com `;` como fim de instrução.
    - Atualização das respostas esperadas dos testes de atribuição (`atribuicao_simples` e `atribuicao_expressao`).
    - Inclusão dos casos `erro_caractere`, `erro_comentario_aberto`, `erro_divisao_zero` e `erro_numero_grande`.
    - **Pendente:** não foram criados testes de atribuição dentro de `if`/`else`, porque a gramática não reconhece essas estruturas.

### 6. Integração da Issue #9: `Tipos e Novos operadores: Atribuição (=) e ponto e virgula (;)`

- **Status:** Concluída (issue do milestone da Semana 4, já descrita em `Semana4.md`)
- **Entrega:** PR #21, que substituiu o PR #20
- **Principais arquivos:** `src/parser.y`, `src/scanner.l`, `src/tabela.h`, `src/makefile`
- **Atividades Desenvolvidas:**
    - O merge deste trabalho na `main` aconteceu em 29/09, durante a Semana 5, e por isso ele é registrado aqui.
    - Adaptação das mensagens de erro da Issue #15 ao `;` como fim de instrução.
    - Padronização dos erros semânticos no formato `Erro Semântico [Linha X, Col Y]: ...`.

## Dificuldades Encontradas e Soluções Adotadas

* **Conflito entre as branches de tipos e de mensagens de erro:** as duas alteravam `scanner.l` e `parser.y` ao mesmo tempo. O PR #20 foi fechado sem merge e substituído pelo PR #21, que manteve `%locations`, `parse.error custom` e os erros léxicos da `main`, agora adaptados ao `;`.
* **Quebra de compatibilidade com a troca de `\n` por `;`:** os testes antigos deixaram de ser válidos. As entradas foram atualizadas no commit `8dc1993`.
* **Build quebrado por arquivos órfãos:** o `scanner.l` incluía `token.tab.h`, que não era gerado pelo `makefile`. O include foi trocado por `parser.tab.h`, e `token.tab.c` e `token.tab.h` foram removidos no commit `c37708b`.
* **Contagem de linhas incorreta:** a regra de espaços também consumia as quebras de linha. As duas regras foram separadas no PR #19.
* **Regras de `if`/`else` não implementadas (pendente):** elas constavam nos objetivos da semana, mas nenhuma issue cobriu a gramática, já que a Issue #16 tratava só do scanner. O `parser.y` declara os tokens, sem regra de derivação para eles.
* **Teste do scanner sem execução (pendente):** o `tests/test_scanner.c` ainda inclui `token.tab.h`, removido nesta semana, então o teste não compila. O `run_scanner_tests.sh` também não é chamado pelo `make test`, e por isso a falha não apareceu.
* **Teste `comentario_multilinha` falhando (resolvido na Semana 6):** a semana terminou com 13 de 14 testes passando. O teste foi corrigido no PR #26.
* **Marcadores de conflito em arquivo gerado (resolvido na Semana 7):** o merge do PR #21 levou `src/lex.yy.c` para a `main` com marcadores `<<<<<<<`. O arquivo é regenerado pelo `make`, então o build não foi afetado. Ele saiu do versionamento no PR #33.

## Próximos Passos (Semana 6)

- **Representação interna do programa, com foco em:**
    - Criação do modelo da AST, com nós tipados, linha de origem, impressão hierárquica e liberação de memória.
    - Evolução da Tabela de Símbolos, com nome, categoria, tipo, escopo e linha, além da detecção de declaração duplicada.
    - Integração entre scanner, parser, AST e Tabela de Símbolos.
    - Testes de expressões, declarações e precedência, com ajustes de qualidade (`-Wall -Wextra`, `.gitignore`).
- **Pendências herdadas desta semana:**
    - Regras de `if`/`else` e blocos na gramática, com os testes da Issue #18.
    - Correção do teste do scanner e sua inclusão no `make test`.
