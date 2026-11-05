# Relatório Semanal - Semana 6

**Projeto:** Compilador (comp1-2026.2-grupo17)

**Disciplina:** Compiladores

## Objetivos da Semana

- Fazer o compilador representar o programa internamente, preparando a análise semântica da Semana 7.
- Criar a AST com nós tipados, linha de origem, impressão hierárquica e liberação de memória.
- Evoluir a Tabela de Símbolos para guardar nome, categoria, tipo, escopo e linha, com detecção de declaração duplicada.
- Integrar scanner, parser, AST e Tabela de Símbolos.
- Ampliar os testes de expressões, declarações e precedência, com ajustes de qualidade (`-Wall -Wextra`, `.gitignore`).
- **Fora do escopo da semana:** verificação de tipos, escopos aninhados, `if`/`else`/`while` na AST e geração de JavaScript.

## Issues Fechadas na Semana 6

### 1. Issue #23: `Criar e melhorar o modelo da AST`

- **Status:** Concluída
- **Entrega:** PR #26 (commits `7e7aaf6` e `c405360`)
- **Principais arquivos:** `src/ast.c`, `src/ast.h`, `src/makefile`, `tests/test_ast.c`, `tests/run_ast_tests.sh`, `tests/ast/esperado.txt`
- **Atividades Desenvolvidas:**
    - Criação da `struct NoAST` e do enum de tipos de nó, com 11 tipos ao final da issue, incluindo `AST_ATRIBUICAO`, `AST_EXPRESSAO` e `AST_MENOS_UNARIO`.
    - Implementação das funções de criação, de `ast_imprimir` (saída indentada) e de `ast_liberar`.
    - Criação de um teste que monta as árvores manualmente, sem depender do parser, compara a saída com o esperado e verifica vazamentos com `valgrind`. Ele cobre o critério de aceite da issue: em `a + b * 2`, a multiplicação fica abaixo da soma.
    - Correção do alvo `make test`, que usava um caminho errado para os scripts.
    - Correção do teste `comentario_multilinha`, levando a branch a 14 de 14 testes passando.
- **Decisões Técnicas:**
    - O valor dos literais usa `union { long i; double r; }` mais o campo `tipo_c`, o que preserva a diferença entre `5` e `5.0` e a precisão de `long`.
    - Todos os construtores recebem linha e coluna, para que as etapas seguintes possam reportar erros na posição correta.
    - `ast_imprimir` e `ast_liberar` percorrem `proximo` com um laço, para que a recursão acompanhe só a profundidade das expressões, e não o número de instruções.

### 2. Issue #24: `Criar a tabela de símbolos`

- **Status:** Concluída
- **Entrega:** PR #25 (commit `204f116`)
- **Principais arquivos:** `src/tabela.c`, `src/tabela.h`, `src/parser.y`, `src/makefile`, `tests/run_tests.sh`, `tests/entradas_tabela/`, `tests/respostas_tabela/`, `tests/entradas_erro/erro_redeclaracao.txt`, `tests/respostas_erro/erro_redeclaracao.txt`
- **Atividades Desenvolvidas:**
    - Separação da tabela em `tabela.h` (interface) e `tabela.c` (implementação), no lugar das funções `static inline` no header.
    - Ampliação do símbolo para guardar nome, categoria, tipo, escopo, linha e coluna.
    - Implementação de `tabela_inserir`, `tabela_buscar`, `tabela_imprimir` e `tabela_liberar`.
    - Erro de redeclaração informando a linha da declaração original.
    - Nova opção `./parser --tabela` (ou `-t`) para imprimir a tabela.
    - Nova seção de testes de tabela no `run_tests.sh`, com verificação de memória por `valgrind` sem vazamentos, conforme o PR.
- **Decisões Técnicas:**
    - O escopo continua apenas global, mas já é parâmetro das funções, para aceitar escopos aninhados sem mudar a interface.
    - A inserção é feita no final da lista, para imprimir os símbolos na ordem de declaração.
    - `valor_int` e `valor_real` continuam no símbolo de forma temporária, porque o parser ainda calcula as expressões. Eles podem sair quando a AST assumir esse papel.
    - As funções antigas (`inserir_variavel`, `buscar_variavel`, `verificar_tipo` e `liberar_tabela`) foram removidas, o que exigiu adaptar o `parser.y`.

### Outras alterações (sem issue)

- Reformatação da seção de integrantes do `README.md` (commit `033d40e`).

## Dificuldades Encontradas e Soluções Adotadas

* **`main` sem compilar após o merge dos PRs #25 e #26:** os dois PRs foram feitos em paralelo e funcionavam isoladamente. O `ast.c` dependia de `TIPO_INEXISTENTE` e `nome_tipo()`, que mudaram na refatoração da tabela. O git não acusou conflito porque as mesmas linhas não foram alteradas, então o erro só apareceu ao compilar (`ast.c:28: error: 'TIPO_INEXISTENTE' undeclared`). A correção veio na Semana 7, pela Issue #27 e pelo PR #33, que levaram a suíte a 18 de 18 testes passando.
* **Prevenção adotada a partir desse problema:** quem revisa deve atualizar a branch com a `main` e rodar `make test` antes de aceitar um PR. Um GitHub Action poderia automatizar essa checagem.
* **Arquivos gerados no versionamento:** `lex.yy.c`, `parser`, `parser.exe` e `relatorio.txt` estavam no repositório e geravam conflitos nos merges. Eles foram removidos do versionamento no PR #33, e cada integrante passou a rodar `make` em `src/` depois do `git pull`.
* **Teste `comentario_multilinha` falhando:** a entrada não tinha `;`, e a resposta esperada estava no formato antigo das mensagens. Os dois arquivos foram atualizados no PR #26.
* **Integração não concluída na semana (movida para a Semana 7):** a AST ficou pronta como módulo, mas o parser ainda não a constrói, e o `make test` não passava na `main` ao fim da semana. Com isso, o critério do milestone (`int a = 10; int b = 20; int soma = a + b * 2;` gerar a AST correta, com `make test` passando 100%) não foi atingido. A integração passou para a Issue #28.

## Próximos Passos (Semana 7)

- **Integração da análise semântica ao compilador:**
    - Correção do build da `main` após o merge da tabela e da AST (Issue #27).
    - Integração da AST ao parser (Issue #28).
    - Módulo semântico com declarações e resolução de nomes (Issue #29).
    - Verificação e propagação de tipos, com `TIPO_ERRO` (Issue #30).
    - Diagnósticos semânticos acumulados e AST anotada (Issue #31).
    - Relatórios das Semanas 5 e 6 (Issue #32).
- **Pendências herdadas da Semana 5, ainda sem issue:**
    - Regras de `if`/`else` e blocos na gramática, com os testes da Issue #18.
    - Correção do teste do scanner e sua inclusão no `make test`.
