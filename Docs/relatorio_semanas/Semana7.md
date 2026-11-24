# Relatório Semanal - Semana 7

**Projeto:** Compilador (comp1-2026.2-grupo17)

**Disciplina:** Compiladores

## Objetivos da Semana

- Recuperar o build da `main` após o merge paralelo da tabela e da AST.
- Integrar a AST ao parser, que deixa de calcular as expressões e passa a apenas construir a árvore.
- Criar a análise semântica como fase separada, com resolução de nomes, verificação e propagação de tipos.
- Acrescentar `if`/`else`, blocos e escopos aninhados à gramática, à AST e à semântica.
- Acumular os diagnósticos semânticos e exibir a AST anotada com tipos e símbolos.
- Publicar a versão `v0.3.0` e apresentar o Ponto de Controle 2 (P2).

## Issues Fechadas na Semana 7

### 1. Issue #27: `Corrigir o build da main após o merge da tabela e da AST`

- **Status:** Concluída
- **Responsável:** Marcus Rezende
- **Entrega:** PR #33 (commit `b10e3d4`)
- **Principais arquivos:** `src/tabela.h`, `tests/run_ast_tests.sh`, `.gitignore`
- **Atividades Desenvolvidas:**
    - Definição de `TIPO_INEXISTENTE` em `tabela.h`, de que o `ast.c` dependia.
    - Inclusão de `tabela.c` na compilação do teste da AST.
    - Remoção de `lex.yy.c`, `parser`, `parser.exe` e `relatorio.txt` do versionamento, com as regras correspondentes no `.gitignore`.
    - Suíte de volta a 100% (18 de 18 testes).

### 2. Issue #28: `Integrar a AST ao parser`

- **Status:** Concluída
- **Responsável:** Miqueias Carvalho, com ajuste de Marcus Rezende
- **Entrega:** PR #34 (commits `800bb59`, `2afef65` e `ce15e20`)
- **Principais arquivos:** `src/parser.y`, `src/scanner.l`, `src/ast.c`, `tests/respostas/`, `tests/run_tests.sh`
- **Atividades Desenvolvidas:**
    - As ações do Bison passam a chamar os construtores `ast_criar_*` no lugar de calcular valores, e a pilha guarda ponteiros para nós (`%union` com `struct NoAST *no`).
    - A raiz fica em `ast_raiz_programa` e é liberada no `main`.
    - Nova opção `./parser --ast` para imprimir a árvore.
    - Respostas dos testes de aritmética atualizadas para a saída da AST.
    - `run_tests.sh` passa a aceitar tanto `parser` (Linux) quanto `parser.exe` (Windows).

### 3. Issue #29: `Módulo de análise semântica com resolução de nomes`

- **Status:** Concluída
- **Responsável:** Marcus Rezende
- **Entrega:** PR #35 (commit `f6f296f`)
- **Principais arquivos:** `src/semantica.c`, `src/semantica.h`, `src/parser.y`, `tests/entradas_erro/`, `tests/respostas_erro/`
- **Atividades Desenvolvidas:**
    - Criação de `semantica.c`, que percorre a AST na ordem do programa depois do parsing.
    - Verificações de redeclaração no mesmo escopo, variável não declarada, uso antes da declaração e autoinicialização (`int x = x;`).
    - A verificação de divisão por literal zero sai do parser e vai para a semântica.
    - Novos testes: `erro_autoinicializacao`, `erro_ordem_semantica`, `erro_uso_antes_declaracao` e `erro_varios_semanticos`.
- **Decisões Técnicas:**
    - A semântica só roda se não houver erro léxico ou sintático, porque uma AST incompleta geraria erros falsos.
    - Na declaração, a inicialização é analisada antes de o nome entrar na tabela, como no C.

### 4. Issue #30: `Verificação e propagação de tipos`

- **Status:** Concluída
- **Responsável:** Maria Laura
- **Entrega:** PR #36 (commit `6da3388`)
- **Principais arquivos:** `src/semantica.c`, `src/semantica.h`, `src/tabela.h`, `README.md`, `tests/entradas_erro/`, `tests/respostas_erro/`
- **Atividades Desenvolvidas:**
    - Inclusão de `TIPO_ERRO` no enum `Tipo`.
    - Anotação de `tipo_c` em todo nó de expressão: o literal mantém o tipo do parser (real é `double`), o identificador recebe o tipo do símbolo, `+ - * /` resultam no tipo mais largo (no mínimo `int`) e o menos unário recebe o tipo do operando (no mínimo `int`).
    - Em atribuições e declarações inicializadas, a conversão de real para inteiro é rejeitada (`atribuição de 'double' para 'int' não permitida em 'a'`).
    - Seis testes novos, um por critério de aceite da issue.
    - Nova seção "Regras de tipos" no `README.md`.
- **Decisões Técnicas:**
    - Estreitamentos entre inteiros (`long → int`) e entre reais (`double → float`) são aceitos, como no C. Só real → inteiro é erro, porque perde a parte fracionária.
    - Um filho com `TIPO_ERRO` torna o pai `TIPO_ERRO` sem gerar nova mensagem, o que evita erros em cascata. Assim, `a = x + 1;` com `x` não declarado gera um único erro.

### 5. Issue #16: `Estruturas de decisão (if/else)`

- **Status:** Concluída (pendência herdada da Semana 5)
- **Responsáveis:** Pedro Henrique e Marcus Rezende
- **Entrega:** PR #37 (commits `e3bedfe` e `bf9b870`)
- **Principais arquivos:** `src/parser.y`, `src/ast.c`, `src/ast.h`, `src/semantica.c`, `src/tabela.c`, `tests/entradas/`, `tests/entradas_erro/`, `tests/test_scanner.c`
- **Atividades Desenvolvidas:**
    - Regras de `if`, `if`/`else` e blocos `{ }` na gramática, com os operadores de comparação (`==`, `!=`, `<`, `<=`, `>`, `>=`).
    - Novos nós `AST_BLOCO`, `AST_IF` e seis nós de comparação, chegando a 19 tipos de nó.
    - Escopos aninhados na tabela de símbolos: cada bloco abre um escopo filho, e a busca vai do escopo atual até o global.
    - A semântica analisa a condição e os dois caminhos do `if`.
    - Testes de `if` com e sem `else`, aninhado e com atribuição, além de escopo fora do bloco, sombreamento e erros na condição e no `else`.
    - O teste do scanner foi corrigido e entrou no `make test`, resolvendo uma pendência da Semana 5.
- **Decisões Técnicas:**
    - A versão inicial do PR executava o programa (`execucao.c`). A execução foi removida para manter o projeto como compilador C → JavaScript, e os testes de `if`/`else` passaram a conferir a AST (`--ast`).

### 6. Issue #31: `Diagnósticos semânticos acumulados e AST anotada`

- **Status:** Concluída (merge em 19/11, já no período da Semana 8)
- **Responsáveis:** Pedro Henrique e Marcus Rezende
- **Entrega:** PR #40 (commits `46230e0` e `3095e61`)
- **Principais arquivos:** `src/ast.c`, `src/ast.h`, `src/parser.y`, `src/semantica.c`, `tests/respostas/`, `tests/respostas_erro/`, `tests/run_tests.sh`
- **Atividades Desenvolvidas:**
    - A semântica percorre o programa inteiro sem parar no primeiro erro e mostra um resumo ao final (`3 erro(s) semântico(s) encontrado(s).`).
    - O `--ast` passa a mostrar o tipo de cada nó (`AST_SOMA [int]`) e o símbolo resolvido (`[símbolo: a, linha 1]`).
    - A AST é impressa mesmo com erros semânticos, com os nós problemáticos anotados como `[erro]`.
    - O resumo de erros foi movido do `stdout` para o `stderr`, para não se misturar à saída do `--ast`.

### 7. Issue #32: `Relatórios das Semanas 5 e 6`

- **Status:** Concluída
- **Responsável:** Edilson Júnior
- **Entrega:** commit `ce4cad8`
- **Principais arquivos:** `Docs/relatorio_semanas/Semana5.md`, `Docs/relatorio_semanas/Semana6.md`

### Outras entregas (sem issue)

- **Console do Windows (PR #38, commit `80ddc63`, Marcus Rezende):** a saída passa a usar UTF-8, corrigindo os acentos das mensagens. O `Ctrl+D`, que no console do Windows chega como o caractere `0x04`, passa a ser tratado como fim da entrada.
- **Versão `v0.3.0` (tag em 09/11, sobre o merge do PR #38):** primeira versão com todas as fases de análise integradas (léxico, sintático, AST e semântico).
- **Apresentação do P2 (11/11):** apresentação da arquitetura em fases, da AST, da análise semântica, dos testes e dos desafios. Cada integrante apresentou dois slides: Marcus (visão do P1 ao P2), Edilson (planejamento e processo), Maria Laura (arquitetura em fases e AST), Miqueias (análise semântica e diagnósticos) e Pedro (desafios e próximos passos).

## Dificuldades Encontradas e Soluções Adotadas

* **`main` sem compilar no início da semana:** herança do merge paralelo dos PRs #25 e #26 (Semana 6). A Issue #27 corrigiu o problema, e o grupo adotou a regra de rodar `make test` com a `main` atualizada antes de cada merge.
* **Execução no lugar de compilação:** a primeira versão do `if`/`else` interpretava o programa. Ela foi refatorada para gerar só a AST e a análise semântica, mantendo o objetivo de traduzir para JavaScript.
* **Diferenças entre Windows e Linux:** acentos quebrados no console, `Ctrl+D` sem efeito e nomes diferentes de executável (`parser` e `parser.exe`). As correções vieram nos PRs #34 e #38.
* **Saída do `--ast` misturada ao resumo de erros:** o resumo ia para o `stdout` junto com a árvore. Ele foi movido para o `stderr` no PR #40.
* **Issue #31 concluída com atraso:** o PR #40 só foi integrado em 19/11, já no período da Semana 8.

## Próximos Passos (Semana 8)

- **Geração de código intermediário (TAC):**
    - TAC para expressões e atribuições (Issue #41).
    - TAC para `if`/`else`, com rótulos e desvios (Issue #42).
    - Laço `while` (Issue #44).
    - Testes automatizados de TAC.
    - Relatórios das Semanas 7 e 8 (Issue #45).
